#include <iostream>
#include <cstdio>
#include <cstring>
#include<bits/stdc++.h>
#include "defs.h"

using namespace std;

struct Student_n {
    string roll;
    float ct_marks;
    float ms_marks;
};
struct Student_d {
    string roll;
    string name;
    float ct_marks;
    float ms_marks;
};

string remove_space(string str)
{
  
    for(int i=0;i+1<str.size();++i)
    {
        if(str[i]==' ' && str[i+1]==' ')
        {

            str[i]='\0';
            break;
        }
    }
    int len = strlen(str.c_str());
    return str.c_str();
}
string toLowerCase(string str) {
    transform(str.begin(), str.end(), str.begin(),
                   [](unsigned char c) { return tolower(c); });
    return str;
}

int main() {
    int option, flag = 0, replace,s_count=0, j_count=0;
    std::ofstream outfile("output.txt");
    cout << "Which replacement technique you want to run: Clock (0) or LRU (1) or MRU (2): ";
    cin>>replace;
    cout << "\nWhich query you want to run: Select (0) or Join (1): ";
    cin>>option;
    string file1="dbms.txt", file2="networks.txt";
    FILE *f1, *f2;
    Page *p, *q;
    int select_flag;


    string input_string;
    if(option==0){
        cout << "\nEnter Name to search for: ";
        getline(cin, input_string);
        getline(cin, input_string);
        f1 = fopen(file1.c_str(), "rb");
        if (!f1) {
            cerr << "Error: could not open data file." << endl;
            return 1;
        }
        f2=f1;
    }
    
    if(option==1)
    {
        
        f1 = fopen(file1.c_str(), "rb");
        if (!f1) {
            cerr << "Error: could not open data file." << endl;
            return 1;
        }
        f2 = fopen(file2.c_str(), "rb");
        if (!f2) {
            cerr << "Error: could not open data file." << endl;
            return 1;
        }

    }
    

    // Create buffer managers
    int num_bufs = 5;
    clock_buffer_manager clock_mgr(num_bufs);
    lru_buffer_manager lru_mgr(num_bufs);
    mru_buffer_manager mru_mgr(num_bufs);

    int i=0;
    // Read pages using clock replacement algorithm
    while(1) 
    {
        //cout << "\nouter loop" <<i<< endl;
        if(replace==0)p = clock_mgr.read_page(f1, i);
        else if (replace==1)p = lru_mgr.read_page(f1, i);
        else p = mru_mgr.read_page(f1, i);

        if (!p) {
            break;
        }

        string data =  reinterpret_cast<char *>(p->disk_block);
        //cout << "Read page " << i << " using clock replacement." << endl;
        //cout << "Page content: \n" << data << endl;
        stringstream ssp(data);
        string line;

        int num_rows = 0;
        while (getline(ssp, line)) {
            if (!line.empty()) {
                num_rows++;
            }
        }
        //cout<<"--"<<num_rows<<endl;
        
        ssp.clear();
        ssp.seekg(0, ios::beg);


        int j = 0;
        Student_d stud;

        int k=0;
        if(option==0)
        {
             while (getline(ssp, line)) {
                //cout<<"\tinner p"<<i<<endl;
                if (!line.empty()) {
                    //cout<<"yes1\n";
                    if(line.size()!=63)continue;
                    stud.name = remove_space(line.substr(22,30));
                    
                    stud.roll = line.substr(8, 9);
                    stud.ct_marks = stof(remove_space(line.substr(51, 7)));
                    stud.ms_marks = stof(remove_space(line.substr(59, 5)));
                    if(option == 0){
                        //cout<<"yes\n";
                        if(s_count==0)
                            {
                                for(int s = 0; s < 80; s++) {
                                    cout << "-";
                                }
                                cout<<endl;
                                cout << setw(29) << left << "             Name" 
                                << setw(15) << left << "Roll Number" 
                                << setw(20) << left << "Class-Test 1 marks" 
                                << setw(10) << left << "Mid-Sem marks" 
                                << endl;
                                for(int s = 0; s < 80; s++) {
                                    cout << "-";
                                }
                                cout<<endl;
                            }
                            s_count++;
                       
                            if(toLowerCase(stud.name).compare(toLowerCase(input_string))==0)
                            {
                                if(option==0)select_flag=1;
                                int name_len = stud.name.length(); // length of the name
                                int name_pad = (30 - name_len) / 2; 
                                cout <<setw(name_pad + name_len) << setfill(' ') << right << stud.name 
                                << setw(name_pad) << setfill(' ') << "" 
                                <<  setw(15) << setfill(' ') << left <<stud.roll 
                                << setw(8) << setfill(' ') << right << stud.ct_marks 
                                << setw(8) << setfill(' ') << ""
                                <<setw(10) << setfill(' ') << right << stud.ms_marks 
                                << setw(7) << setfill(' ') << "" 
                                << endl;
                                for(int s = 0; s < 80; s++) {
                                        cout << "-";
                                    }
                                cout<<endl;
                                flag=1;
                                break;
                            }
                    }
                }
            }
        }
        else{
            while(1)
        {
            
            if(replace==0) q = clock_mgr.read_page(f2, k);
            else if (replace==1) q = lru_mgr.read_page(f2, k);
            else q = mru_mgr.read_page(f2, k);
            if (!q) {
                break;
            }

            string q_data =  reinterpret_cast<char *>(q->disk_block);
            stringstream ss(q_data);
            string q_line;

            int q_num_rows = 0;
            while (getline(ss, q_line)) {
                if (!q_line.empty()) {
                    q_num_rows++;
                }
            }

            ss.clear();
            ss.seekg(0, ios::beg);

            Student_n q_stud;
            if(option==1){
                if(j_count==0)
                {
                    for(int s = 0; s < 120; s++) {
                        outfile << "-";
                    }
                    outfile<<endl;
                    outfile << setw(34) << left << "                Name" 
                    << setw(15) << left << "Roll Number" 
                    << setw(20) << left << "Networks CT-1" 
                    << setw(10) << left << "Networks Mid-Sem"
                    << setw(20) << left << "  DBMS CT-1" 
                    << setw(10) << left << "DBMS Mid-Sem" 
                    << endl;
                    for(int s = 0; s < 120; s++) {
                        outfile << "-";
                    }
                    outfile<<endl;
                }
            }
            j_count++;
                        
            while (getline(ssp, line)) {
                //cout<<"\tinner p"<<i<<endl;
                if (!line.empty()) {
                    //cout<<"yes1\n";
                    if(line.size()!=63)continue;
                    stud.name = remove_space(line.substr(22,30));
                    
                    stud.roll = line.substr(8, 9);
                    stud.ct_marks = stof(remove_space(line.substr(51, 7)));
                    stud.ms_marks = stof(remove_space(line.substr(59, 5)));
                    
                    if(option==1){
                        while (getline(ss, q_line)) {
                                //cout<<"\t\tinner q"<<k<<endl;
                                if (!q_line.empty()) {
                                    q_stud.roll = q_line.substr(4, 9);
                                    q_stud.ms_marks = stof(remove_space(q_line.substr(16, 5)));
                                    q_stud.ct_marks = stof(remove_space(q_line.substr(25, 4)));
                                    if(q_stud.roll.compare(stud.roll)==0)
                                    {
                                        outfile<<"\n";
                                        int name_len = stud.name.length(); // length of the name
                                        int name_pad = (35 - name_len) / 2; 
                                        outfile <<setw(name_pad + name_len) << setfill(' ') << right << stud.name 
                                        << setw(name_pad) << setfill(' ') << "" 
                                        <<  setw(15) << setfill(' ') << left <<stud.roll 
                                        << setw(8) << setfill(' ') << right << q_stud.ct_marks 
                                        << setw(8) << setfill(' ') << ""
                                        <<setw(10) << setfill(' ') << right << q_stud.ms_marks 
                                        << setw(7) << setfill(' ') << ""
                                        << setw(8) << setfill(' ') << right << stud.ct_marks 
                                        << setw(8) << setfill(' ') << ""
                                        <<setw(10) << setfill(' ') << right << stud.ms_marks 
                                        << setw(7) << setfill(' ') << "" 
                                        << endl;
                                        for(int s = 0; s < 120; s++) {
                                            outfile << "-";
                                        }
                                        outfile<<endl;
                                        flag=1;
                                    }
                                    else{
                                        continue;
                                    }
                                
                                }
                                
                            }
                            ss.clear();
                            ss.seekg(0, ios::beg);
                            
                        }
                    }
                }
                 if(select_flag==1)break;
                ssp.clear();
                ssp.seekg(0, ios::beg);
            if(replace == 0)clock_mgr.unpin_page(q);
            else if(replace == 1)lru_mgr.unpin_page(q);
            else mru_mgr.unpin_page(q);
            k++;
        }
        }
        if(replace==0)clock_mgr.unpin_page(p);
        else if(replace==1)lru_mgr.unpin_page(p);
        else mru_mgr.unpin_page(p);

        i++;
    }
    if (flag==0)
    {
        if(option==0)
        {
            cout <<setw(50) << setfill(' ') << right << "No matching entry found"
            << setw(20) << setfill(' ') << "" <<endl;
            for(int s = 0; s < 80; s++) {
                cout << "-";
            }
            cout<<endl;
        }
        if(option==1)
        {
            cout <<setw(50) << setfill(' ') << right << "No matching entry found"
            << setw(50) << setfill(' ') << "" <<endl;
            for(int s = 0; s < 120; s++) {
                cout << "-";
            }
            cout<<endl;
        }
    }

    fclose(f1);
    if(option==1)
    {   
        cout<<"Output written to output.txt\n";
        fclose(f2);
    }

    if(replace==0)
    {
        cout<<"Number of Buffer hits: "<<clock_mgr.accesses<<endl;
        cout<<"Number of Disk I/O i.e. buffer miss: "<< clock_mgr.disk_reads<<endl;
    }
    if(replace==1)
    {
        cout<<"Number of Buffer hits: "<< lru_mgr.accesses<< endl;
        cout<<"Number of Disk I/O i.e. buffer miss: "<<lru_mgr.disk_reads<<endl;
    }
    if(replace==2)
    {
        cout<<"Number of Buffer hits: "<< mru_mgr.accesses<<endl;
        cout<<"Number of Disk I/O i.e. buffer miss: "<< mru_mgr.disk_reads<<endl;
    }
    outfile.close();
    return 0;
}
