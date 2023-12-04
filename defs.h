#ifndef DEFS_H
#define DEFS_H

#include <unordered_map>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <list>
using namespace std;

#define PAGE_SIZE 512

class Page;

struct hash_struct{
    unsigned long operator()(const pair<FILE*, int>& p) const {
        const size_t file_hash = hash<FILE*>{}(p.first);
        const size_t int_hash = hash<int>{}(p.second);
        return file_hash ^ (int_hash << 1);
    }
};

class clock_buffer_manager{
	private:
		FILE *log_ptr;
		int clock_hand;
		unsigned int num_bufs;
		int buf_cnt;
		unordered_map <pair<FILE*,int>,int,hash_struct> buf_map;
		Page *buf_pool;
		int replace_page(FILE *f,int page_number);
	public:
		int accesses,disk_reads;
		clock_buffer_manager(unsigned int n);
		~clock_buffer_manager();
		Page *read_page(FILE *f, int page_number);
		void unpin_page(Page *p);
};

class lru_buffer_manager{
	private:
		FILE *log_ptr;
		unsigned int num_bufs;
		int buf_cnt;
		list <Page*> buf_pool;
		unordered_map <pair<FILE*,int>,list<Page*>::iterator,hash_struct> buf_map;
	public:
		int accesses, disk_reads;
		lru_buffer_manager(unsigned int n);
		~lru_buffer_manager();
		Page *read_page(FILE *f,int page_number);
		void unpin_page(Page* p);
};

class mru_buffer_manager{
	private:
		FILE *log_ptr;
		unsigned int num_bufs;
		int buf_cnt;
		list <Page*> buf_pool;
		unordered_map <pair<FILE*,int>,list<Page*>::iterator,hash_struct> buf_map;
	public:
		int accesses, disk_reads;
		mru_buffer_manager(unsigned int n);
		~mru_buffer_manager();
		Page *read_page(FILE *f,int page_number);
		void unpin_page(Page* p);
};

class Page{
	private:
		FILE *fptr;
		int page_num;
		bool ref_bit;
		bool pin;
		Page();
		~Page();
	public:
		void *disk_block;
		friend class clock_buffer_manager;
		friend class lru_buffer_manager;
		friend class mru_buffer_manager;
};

#endif