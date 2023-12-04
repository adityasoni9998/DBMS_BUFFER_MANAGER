#include "defs.h"
#include <iostream>
#include <cstring>

Page::Page():fptr(NULL),page_num(-1),ref_bit(false),pin(false){
	disk_block = malloc(PAGE_SIZE);
}

Page::~Page(){
	free(disk_block);
}

clock_buffer_manager::clock_buffer_manager(unsigned int n):
clock_hand(n-1),num_bufs(n),buf_cnt(0),accesses(0),disk_reads(0){
	buf_pool = new Page[n];
	log_ptr = fopen("log_clock.txt","w");
}

clock_buffer_manager::~clock_buffer_manager(){
	delete[] buf_pool;
	fclose(log_ptr);
}

void clock_buffer_manager::unpin_page(Page* p){
	p->pin = false;
}

//clock replacement algorithm
int clock_buffer_manager::replace_page(FILE*f,int page_number){
	for(int i=0;i<(int)(2*num_bufs);i++){
		clock_hand = (clock_hand + 1)%num_bufs;
		if(buf_pool[clock_hand].ref_bit == 1){
			buf_pool[clock_hand].ref_bit = 0;
			continue;
		}
		else{
			if(buf_pool[clock_hand].pin == 1){
				continue;
			}
			else{
				int r = fseek(f,page_number*PAGE_SIZE,SEEK_SET);
				if(r<0)return -1;
				r = fread(buf_pool[clock_hand].disk_block,PAGE_SIZE,1,f);
				if(r!=1)return -1;
				return clock_hand;
			}
		}
	}
	return -1;
}

//zero-indexed page number
//returns NULL on error
Page *clock_buffer_manager::read_page(FILE *f,int page_number){
	if(f==NULL || page_number<0)return NULL;
	pair <FILE *,int> key = make_pair(f,page_number);
	int idx;
	if(buf_map.find(key) != buf_map.end()){
		accesses++;
		idx = buf_map[key];
		buf_pool[idx].pin = buf_pool[idx].ref_bit = 1;
		fprintf(log_ptr,"Found page page no.: %d for FILE: %p in buffer pool at index %d\n",page_number,f,idx);
	}
	else if(buf_cnt<(int)num_bufs){
		int r = fseek(f,page_number*PAGE_SIZE,SEEK_SET);
		if(r<0)return NULL;
		r = fread(buf_pool[buf_cnt].disk_block,PAGE_SIZE,1,f);
		if(r!=1)return NULL;
		disk_reads++;
		idx = buf_cnt;
		buf_map[key] = idx;
		buf_pool[idx].pin = buf_pool[idx].ref_bit = 1;		
		buf_pool[idx].fptr = f;
		buf_pool[idx].page_num = page_number;
		buf_cnt++;
		fprintf(log_ptr,"Could not find page in buffer pool but the pool has free space. Read page no.: %d from for FILE: %p into buffer pool at index %d\n",page_number,f,idx);
	}
	else{
		idx = replace_page(f,page_number);
		if(idx==-1)return NULL;
		fprintf(log_ptr,"Evicted page stored at index: %d. Storing page no.: %d of FILE: %p at this index in buffer pool.\n",idx,page_number,f);
		disk_reads++;
		auto it = make_pair(buf_pool[idx].fptr,buf_pool[idx].page_num);
		buf_map.erase(it);
		buf_map[key] = idx;
		
		buf_pool[idx].pin = buf_pool[idx].ref_bit = 1;
		buf_pool[idx].fptr = f;
		buf_pool[idx].page_num = page_number;		
	}
	return &(buf_pool[idx]);
}

lru_buffer_manager::lru_buffer_manager(unsigned int n):
num_bufs(n),buf_cnt(0), accesses(0),disk_reads(0){
	buf_pool.clear();
	log_ptr = fopen("log_lru.txt","w");
}

lru_buffer_manager::~lru_buffer_manager(){
	for(auto page:buf_pool){
		if(page!=NULL)delete page;
	}
	fclose(log_ptr);
}

void lru_buffer_manager::unpin_page(Page* p){
	p->pin = false;
	return;
}

Page* lru_buffer_manager::read_page(FILE *fptr,int page_number){
	if(fptr==NULL || page_number<0)return NULL;
	pair<FILE *, int> key = make_pair(fptr,page_number);
	if(buf_map.find(key)!=buf_map.end()){
		accesses++;
		list <Page*>::iterator it = buf_map[key];
		Page *p = *it;
		p->pin = 1;
		buf_pool.erase(it);

		buf_pool.push_front(p);
		buf_map[key] = buf_pool.begin();
		fprintf(log_ptr,"Found page page no.: %d for FILE: %p in buffer pool.\n",page_number,fptr);
		return p;
	}
	else if(buf_cnt<(int)num_bufs){
		int r = fseek(fptr,page_number*PAGE_SIZE,SEEK_SET);
		if(r<0)return NULL;
		Page *x = new Page;
		x->fptr = fptr;
		x->page_num = page_number;
		x->pin = 1;
		r = fread(x->disk_block,PAGE_SIZE,1,fptr);
		if(r!=1){
			delete x;
			return NULL;
		}
		buf_cnt++;	
		buf_pool.push_front(x);
		buf_map[key] = buf_pool.begin();
		disk_reads++;
		fprintf(log_ptr,"Could not find page in buffer pool but the pool has free space. Read page no.: %d from for FILE: %p into buffer pool\n",page_number,fptr);
		return x;
	}
	else{
		list <Page*>::iterator it = buf_pool.end();
		it--;
		while(1){
			Page *x = *it;
			if(x->pin == 1){
				if(it==buf_pool.begin())return NULL;
				else{
					it--;
				}
			}	
			else break;
		}
		Page *x = *it;
		int r = fseek(fptr,page_number*PAGE_SIZE,SEEK_SET);
		if(r<0)return NULL;

		void *temp = malloc(PAGE_SIZE);
		memcpy(temp,x->disk_block,PAGE_SIZE);
		
		r = fread(x->disk_block,PAGE_SIZE,1,fptr);
		if(r!=1){
			memcpy(x->disk_block,temp,PAGE_SIZE);
			free(temp);
			return NULL;
		}
		free(temp);
		fprintf(log_ptr,"Evicted page no.: %d of File: %p from buffer pool. Read new page no.: %d from FILE: %p into the pool.\n",x->page_num,x->fptr,page_number,fptr);
		buf_map.erase(make_pair(x->fptr,x->page_num));
		buf_pool.erase(it);
		
		x->pin = 1;
		x->fptr = fptr;
		x->page_num = page_number;
		
		buf_pool.push_front(x);
		buf_map[make_pair(x->fptr,x->page_num)] = buf_pool.begin();
		
		disk_reads++;	
		return x;
	}
}

mru_buffer_manager::mru_buffer_manager(unsigned int n):
num_bufs(n),buf_cnt(0), accesses(0),disk_reads(0){
	buf_pool.clear();
	log_ptr = fopen("log_mru.txt","w");
}

mru_buffer_manager::~mru_buffer_manager(){
	for(auto page:buf_pool){
		if(page!=NULL)delete page;
	}
	fclose(log_ptr);
}

void mru_buffer_manager::unpin_page(Page* p){
	p->pin = false;
	return;
}

Page* mru_buffer_manager::read_page(FILE *fptr,int page_number){
	if(fptr==NULL || page_number<0)return NULL;
	pair<FILE *, int> key = make_pair(fptr,page_number);
	if(buf_map.find(key)!=buf_map.end()){
		accesses++;
		list <Page*>::iterator it = buf_map[key];
		Page *p = *it;
		p->pin = 1;
		buf_pool.erase(it);

		buf_pool.push_front(p);
		buf_map[key] = buf_pool.begin();
		fprintf(log_ptr,"Found page no.: %d for FILE: %p in buffer pool\n",page_number,fptr);
		return p;
	}
	else if(buf_cnt<(int)num_bufs){
		int r = fseek(fptr,page_number*PAGE_SIZE,SEEK_SET);
		if(r<0)return NULL;
		Page *x = new Page;
		x->fptr = fptr;
		x->page_num = page_number;
		x->pin = 1;
		r = fread(x->disk_block,PAGE_SIZE,1,fptr);
		if(r!=1){
			delete x;
			return NULL;
		}
		buf_cnt++;	
		buf_pool.push_front(x);
		buf_map[key] = buf_pool.begin();
		disk_reads++;
		fprintf(log_ptr,"Could not find page in buffer pool but the pool has free space. Read page no.: %d for FILE: %p into buffer pool\n",page_number,fptr);
		return x;
	}
	else{
		list <Page*>::iterator it = buf_pool.begin();
		while(it!=buf_pool.end()){
			Page *x = *it;
			if(x->pin==1)it++;
			else{
				break;
			}
		}
		if(it==buf_pool.end())return NULL;
		Page *x = *it;
		int r = fseek(fptr,page_number*PAGE_SIZE,SEEK_SET);
		if(r<0)return NULL;

		void *temp = malloc(PAGE_SIZE);
		memcpy(temp,x->disk_block,PAGE_SIZE);
		
		r = fread(x->disk_block,PAGE_SIZE,1,fptr);
		if(r!=1){
			memcpy(x->disk_block,temp,PAGE_SIZE);
			free(temp);
			return NULL;
		}
		free(temp);
		fprintf(log_ptr,"Evicted page no.: %d of File: %p from buffer pool. Read new page no.: %d from FILE: %p into the pool.\n",x->page_num,x->fptr,page_number,fptr);
		buf_map.erase(make_pair(x->fptr,x->page_num));
		buf_pool.erase(it);
		
		x->pin = 1;
		x->fptr = fptr;
		x->page_num = page_number;
		
		buf_pool.push_front(x);
		buf_map[make_pair(x->fptr,x->page_num)] = buf_pool.begin();
		
		disk_reads++;	
		return x;
	}
}