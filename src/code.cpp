#include <bits/stdc++.h>
using namespace std;

struct Account { char userid[31]; char password[31]; char username[31]; int privilege; };
struct Book { char isbn[21]; char name[61]; char author[61]; char keyword[61]; double price; int stock; };
struct FinanceRecord { double income; double expense; };

static const string ACC_FILE = "accounts.dat";
static const string BOOK_FILE = "books.dat";
static const string FIN_FILE = "finance.dat";
static const string OPS_LOG = "ops.log";

template<size_t N>
static void strcpy_safe(char (&dst)[N], const string &src){ memset(dst, 0, N); strncpy(dst, src.c_str(), N-1); dst[N-1]=0; }

static string normalize_spaces(const string &s){ string out; out.reserve(s.size()); bool in_space=false; for(char c: s){ if(c==' ') { if(!in_space){ out.push_back(' '); in_space=true; } } else { out.push_back(c); in_space=false; } } if(!out.empty() && out.front()==' ') out.erase(out.begin()); if(!out.empty() && out.back()==' ') out.pop_back(); return out; }

static bool is_valid_userid(const string &s){ if(s.size()>30||s.empty()) return false; for(char c:s){ if(!(isalnum((unsigned char)c)||c=='_')) return false; } return true; }
static bool is_valid_password(const string &s){ return is_valid_userid(s); }
static bool is_valid_username(const string &s){ if(s.empty()||s.size()>30) return false; for(unsigned char c: s){ if(c<32) return false; } return true; }
static bool is_digit_str(const string &s){ if(s.empty()) return false; for(char c:s){ if(!isdigit((unsigned char)c)) return false; } return true; }
static bool is_valid_priv(const string &s){ return s=="1"||s=="3"||s=="7"; }

static bool is_valid_isbn(const string &s){ if(s.size()>20) return false; for(unsigned char c:s){ if(c<32) return false; } return true; }
static bool is_valid_name_author(const string &s){ if(s.size()>60) return false; for(unsigned char c:s){ if(c<32 || c=='"') return false; } return true; }
static bool is_valid_keyword(const string &s){ if(s.size()>60) return false; for(unsigned char c:s){ if(c<32 || c=='"') return false; } return true; }
static bool keyword_has_multi(const string &s){ return s.find('|')!=string::npos; }

static bool is_valid_price_str(const string &s){ if(s.empty()||s.size()>13) return false; int dots=0; for(char c:s){ if(c=='.') dots++; else if(!isdigit((unsigned char)c)) return false; } return dots<=1; }
static bool is_positive_int(const string &s){ if(!is_digit_str(s)) return false; long long v=0; for(char c: s){ v = v*10 + (c-'0'); if(v>INT_MAX) return false; } return v>0; }
static bool is_positive_number(const string &s){ if(!is_valid_price_str(s)) return false; double v; try{ v=stod(s); } catch(...) { return false; } return v>0.0; }

static void ensure_root_account(){ fstream f(ACC_FILE, ios::in|ios::binary); bool need_create=false, has_root=false; if(!f.good()) need_create=true; else { Account a; while(f.read(reinterpret_cast<char*>(&a), sizeof(a))){ if(string(a.userid)=="root") { has_root=true; break; } } } f.close(); if(need_create || !has_root){ fstream out(ACC_FILE, ios::out|ios::app|ios::binary); Account root{}; strcpy_safe(root.userid, "root"); strcpy_safe(root.password, "sjtu"); strcpy_safe(root.username, "root"); root.privilege=7; out.write(reinterpret_cast<char*>(&root), sizeof(root)); out.close(); } }

static optional<Account> find_account(const string &userid){ fstream f(ACC_FILE, ios::in|ios::binary); if(!f.good()) return nullopt; Account a; while(f.read(reinterpret_cast<char*>(&a), sizeof(a))){ if(string(a.userid)==userid) { f.close(); return a; } } f.close(); return nullopt; }

static bool upsert_account(const Account &acc, bool must_new){ vector<Account> all; fstream f(ACC_FILE, ios::in|ios::binary); Account a; if(f.good()) { while(f.read(reinterpret_cast<char*>(&a), sizeof(a))) all.push_back(a); } f.close(); int idx=-1; for(int i=0;i<(int)all.size();++i){ if(string(all[i].userid)==string(acc.userid)) { idx=i; break;} } if(must_new){ if(idx!=-1) return false; all.push_back(acc); } else { if(idx==-1) all.push_back(acc); else all[idx]=acc; } fstream out(ACC_FILE, ios::out|ios::binary|ios::trunc); for(auto &x: all) out.write(reinterpret_cast<char*>(&x), sizeof(x)); out.close(); return true; }

static bool delete_account(const string &userid){ vector<Account> all; fstream f(ACC_FILE, ios::in|ios::binary); Account a; if(f.good()) { while(f.read(reinterpret_cast<char*>(&a), sizeof(a))) all.push_back(a); } f.close(); int idx=-1; for(int i=0;i<(int)all.size();++i){ if(string(all[i].userid)==userid) { idx=i; break;} } if(idx==-1) return false; all.erase(all.begin()+idx); fstream out(ACC_FILE, ios::out|ios::binary|ios::trunc); for(auto &x: all) out.write(reinterpret_cast<char*>(&x), sizeof(x)); out.close(); return true; }

static optional<Book> find_book(const string &isbn){ fstream f(BOOK_FILE, ios::in|ios::binary); if(!f.good()) return nullopt; Book b; while(f.read(reinterpret_cast<char*>(&b), sizeof(b))){ if(string(b.isbn)==isbn) { f.close(); return b; } } f.close(); return nullopt; }

static bool upsert_book(const Book &book){ vector<Book> all; fstream f(BOOK_FILE, ios::in|ios::binary); Book b; if(f.good()) { while(f.read(reinterpret_cast<char*>(&b), sizeof(b))) all.push_back(b); } f.close(); int idx=-1; for(int i=0;i<(int)all.size();++i){ if(string(all[i].isbn)==string(book.isbn)) { idx=i; break;} } if(idx==-1) all.push_back(book); else all[idx]=book; sort(all.begin(), all.end(), [](const Book &x, const Book &y){ return string(x.isbn)<string(y.isbn);} ); fstream out(BOOK_FILE, ios::out|ios::binary|ios::trunc); for(auto &x: all) out.write(reinterpret_cast<char*>(&x), sizeof(x)); out.close(); return true; }

static vector<Book> query_books(function<bool(const Book&)> pred){ vector<Book> ans; fstream f(BOOK_FILE, ios::in|ios::binary); Book b; if(f.good()) { while(f.read(reinterpret_cast<char*>(&b), sizeof(b))) if(pred(b)) ans.push_back(b); } f.close(); sort(ans.begin(), ans.end(), [](const Book &x, const Book &y){ return string(x.isbn)<string(y.isbn);} ); return ans; }

static void record_finance(double income, double expense){ fstream f(FIN_FILE, ios::out|ios::app|ios::binary); FinanceRecord r{income, expense}; f.write(reinterpret_cast<char*>(&r), sizeof(r)); f.close(); }
static vector<FinanceRecord> read_finance(){ vector<FinanceRecord> v; fstream f(FIN_FILE, ios::in|ios::binary); FinanceRecord r; if(f.good()) { while(f.read(reinterpret_cast<char*>(&r), sizeof(r))) v.push_back(r);} f.close(); return v; }

struct Session { Account acc; string selected_isbn; };
static vector<Session> login_stack;
static int current_priv(){ if(login_stack.empty()) return 0; return login_stack.back().acc.privilege; }
static void log_op(const string &line){ fstream f(OPS_LOG, ios::out|ios::app); f << line << "\n"; }
static void print_invalid(){ cout << "Invalid\n"; }

int main(){ ios::sync_with_stdio(false); cin.tie(nullptr); ensure_root_account(); string line;
    while(true){
        if(!getline(cin, line)) break;
        bool only_spaces=true; for(char c: line){ if(c!=' ') { only_spaces=false; break; } }
        if(only_spaces) continue;
        if(!line.empty() && line.back()=='\r') line.pop_back();
        line=normalize_spaces(line);
        if(line.empty()) continue;

        // handle show finance first
        if(line.rfind("show finance", 0)==0){
            if(current_priv()<7){ print_invalid(); }
            else {
                string rest=line.substr(12); rest=normalize_spaces(rest);
                auto v=read_finance();
                if(rest.empty()){
                    double inc=0, exp=0; for(auto &r:v){ inc+=r.income; exp+=r.expense; }
                    cout<<"+ "<<fixed<<setprecision(2)<<inc<<" - "<<fixed<<setprecision(2)<<exp<<"\n";
                } else if(!is_digit_str(rest)){
                    print_invalid();
                } else {
                    long long cnt=stoll(rest);
                    if(cnt==0){ cout<<"\n"; }
                    else if(cnt>(long long)v.size()){ print_invalid(); }
                    else { double inc=0, exp=0; for(long long i=(long long)v.size()-cnt;i<(long long)v.size();++i){ inc+=v[i].income; exp+=v[i].expense; } cout<<"+ "<<fixed<<setprecision(2)<<inc<<" - "<<fixed<<setprecision(2)<<exp<<"\n"; }
                }
            }
            log_op(line);
            continue;
        }

        // general dispatch
        string cmd; { size_t sp=line.find(' '); cmd=(sp==string::npos? line: line.substr(0, sp)); }
        if(cmd=="quit"||cmd=="exit"){ break; }
        else if(cmd=="su"){
            vector<string> tok; string tmp=line; size_t pos=0; while(true){ size_t sp=tmp.find(' ', pos); if(sp==string::npos){ tok.push_back(tmp.substr(pos)); break; } tok.push_back(tmp.substr(pos, sp-pos)); pos=sp+1; }
            if(tok.size()<2||tok.size()>3){ print_invalid(); }
            else {
                string uid=tok[1]; if(!is_valid_userid(uid)){ print_invalid(); }
                else {
                    auto accopt=find_account(uid); if(!accopt){ print_invalid(); }
                    else {
                        string pwd = tok.size()==3 ? tok[2] : string(); bool ok=false;
                        if(!pwd.empty()){ if(!is_valid_password(pwd)){ print_invalid(); } else ok=(string(accopt->password)==pwd); }
                        else { ok = current_priv()>accopt->privilege; }
                        if(!ok){ print_invalid(); }
                        else { Session s; s.acc=*accopt; s.selected_isbn.clear(); login_stack.push_back(s); }
                    }
                }
            }
        }
        else if(cmd=="logout"){
            if(current_priv()<1){ print_invalid(); }
            else if(login_stack.empty()){ print_invalid(); }
            else login_stack.pop_back();
        }
        else if(cmd=="register"){
            vector<string> tok; string tmp=line; size_t pos=0; while(true){ size_t sp=tmp.find(' ', pos); if(sp==string::npos){ tok.push_back(tmp.substr(pos)); break; } tok.push_back(tmp.substr(pos, sp-pos)); pos=sp+1; }
            if(tok.size()!=4){ print_invalid(); }
            else {
                string uid=tok[1], pwd=tok[2], uname=tok[3];
                if(!is_valid_userid(uid)||!is_valid_password(pwd)||!is_valid_username(uname)){ print_invalid(); }
                else { Account a{}; strcpy_safe(a.userid, uid); strcpy_safe(a.password, pwd); strcpy_safe(a.username, uname); a.privilege=1; if(!upsert_account(a, true)){ print_invalid(); } }
            }
        }
        else if(cmd=="passwd"){
            if(current_priv()<1){ print_invalid(); }
            else {
                vector<string> tok; string tmp=line; size_t pos=0; while(true){ size_t sp=tmp.find(' ', pos); if(sp==string::npos){ tok.push_back(tmp.substr(pos)); break; } tok.push_back(tmp.substr(pos, sp-pos)); pos=sp+1; }
                if(tok.size()!=3 && tok.size()!=4){ print_invalid(); }
                else {
                    string uid=tok[1]; auto accopt=find_account(uid); if(!accopt){ print_invalid(); }
                    else {
                        string newpwd = tok.size()==3 ? tok[2] : tok[3];
                        if(!is_valid_password(newpwd)){ print_invalid(); }
                        else if(tok.size()==4){ string curpwd = tok[2]; if(!is_valid_password(curpwd) || string(accopt->password)!=curpwd){ print_invalid(); }
                            else { Account a=*accopt; strcpy_safe(a.password, newpwd); if(!upsert_account(a, false)) print_invalid(); }
                        } else { if(current_priv()!=7){ print_invalid(); } else { Account a=*accopt; strcpy_safe(a.password, newpwd); if(!upsert_account(a, false)) print_invalid(); } }
                    }
                }
            }
        }
        else if(cmd=="useradd"){
            if(current_priv()<3){ print_invalid(); }
            else {
                vector<string> tok; string tmp=line; size_t pos=0; while(true){ size_t sp=tmp.find(' ', pos); if(sp==string::npos){ tok.push_back(tmp.substr(pos)); break; } tok.push_back(tmp.substr(pos, sp-pos)); pos=sp+1; }
                if(tok.size()!=5){ print_invalid(); }
                else {
                    string uid=tok[1], pwd=tok[2], privs=tok[3], uname=tok[4];
                    if(!is_valid_userid(uid)||!is_valid_password(pwd)||!is_valid_priv(privs)||!is_valid_username(uname)){ print_invalid(); }
                    else { int p=stoi(privs); if(p>=current_priv()){ print_invalid(); }
                        else { Account a{}; strcpy_safe(a.userid, uid); strcpy_safe(a.password, pwd); strcpy_safe(a.username, uname); a.privilege=p; if(!upsert_account(a, true)){ print_invalid(); } }
                    }
                }
            }
        }
        else if(cmd=="delete"){
            if(current_priv()<7){ print_invalid(); }
            else {
                vector<string> tok; string tmp=line; size_t pos=0; while(true){ size_t sp=tmp.find(' ', pos); if(sp==string::npos){ tok.push_back(tmp.substr(pos)); break; } tok.push_back(tmp.substr(pos, sp-pos)); pos=sp+1; }
                if(tok.size()!=2){ print_invalid(); }
                else { string uid=tok[1]; if(!is_valid_userid(uid)){ print_invalid(); }
                    else { bool logged=false; for(auto &s: login_stack){ if(string(s.acc.userid)==uid){ logged=true; break; } }
                        if(logged){ print_invalid(); }
                        else if(!delete_account(uid)){ print_invalid(); }
                    }
                }
            }
        }
        else if(cmd=="show"){
            if(current_priv()<1){ print_invalid(); }
            else {
                string arg = (line.size()>5? line.substr(5):""); arg=normalize_spaces(arg);
                if(arg.empty()){
                    auto v=query_books([](const Book&){return true;});
                    if(v.empty()) cout<<"\n"; else for(auto &b: v){ cout<<b.isbn<<"\t"<<b.name<<"\t"<<b.author<<"\t"<<b.keyword<<"\t"<<fixed<<setprecision(2)<<b.price<<"\t"<<b.stock<<"\n"; }
                } else {
                    auto fail=[&](){ print_invalid(); };
                    if(arg.rfind("-ISBN=",0)==0){ string v=arg.substr(6); if(v.empty()||!is_valid_isbn(v)){ fail(); }
                        else { auto res=query_books([&](const Book &b){ return string(b.isbn)==v; }); if(res.empty()) cout<<"\n"; else for(auto &b: res){ cout<<b.isbn<<"\t"<<b.name<<"\t"<<b.author<<"\t"<<b.keyword<<"\t"<<fixed<<setprecision(2)<<b.price<<"\t"<<b.stock<<"\n"; } }
                    }
                    else if(arg.rfind("-name=\"",0)==0 && arg.size()>=8 && arg.back()=='"'){
                        string v=arg.substr(7, arg.size()-8); if(v.empty()||!is_valid_name_author(v)){ fail(); }
                        else { auto res=query_books([&](const Book &b){ return string(b.name)==v; }); if(res.empty()) cout<<"\n"; else for(auto &b: res){ cout<<b.isbn<<"\t"<<b.name<<"\t"<<b.author<<"\t"<<b.keyword<<"\t"<<fixed<<setprecision(2)<<b.price<<"\t"<<b.stock<<"\n"; } }
                    }
                    else if(arg.rfind("-author=\"",0)==0 && arg.size()>=10 && arg.back()=='"'){
                        string v=arg.substr(9, arg.size()-10); if(v.empty()||!is_valid_name_author(v)){ fail(); }
                        else { auto res=query_books([&](const Book &b){ return string(b.author)==v; }); if(res.empty()) cout<<"\n"; else for(auto &b: res){ cout<<b.isbn<<"\t"<<b.name<<"\t"<<b.author<<"\t"<<b.keyword<<"\t"<<fixed<<setprecision(2)<<b.price<<"\t"<<b.stock<<"\n"; } }
                    }
                    else if(arg.rfind("-keyword=\"",0)==0 && arg.size()>=12 && arg.back()=='"'){
                        string v=arg.substr(11, arg.size()-12); if(v.empty()||!is_valid_keyword(v) || keyword_has_multi(v)){ fail(); }
                        else { auto res=query_books([&](const Book &b){ string kw(b.keyword); vector<string> segs; string cur; for(char c: kw){ if(c=='|'){ if(!cur.empty()) segs.push_back(cur); cur.clear(); } else cur.push_back(c);} if(!cur.empty()) segs.push_back(cur); for(string &s: segs){ if(s==v) return true; } return false; }); if(res.empty()) cout<<"\n"; else for(auto &b: res){ cout<<b.isbn<<"\t"<<b.name<<"\t"<<b.author<<"\t"<<b.keyword<<"\t"<<fixed<<setprecision(2)<<b.price<<"\t"<<b.stock<<"\n"; } }
                    }
                    else { fail(); }
                }
            }
        }
        else if(cmd=="buy"){
            if(current_priv()<1){ print_invalid(); }
            else {
                vector<string> tok; string tmp=line; size_t pos=0; while(true){ size_t sp=tmp.find(' ', pos); if(sp==string::npos){ tok.push_back(tmp.substr(pos)); break; } tok.push_back(tmp.substr(pos, sp-pos)); pos=sp+1; }
                if(tok.size()!=3){ print_invalid(); }
                else { string isbn=tok[1], qtys=tok[2]; if(!is_valid_isbn(isbn)||!is_positive_int(qtys)){ print_invalid(); }
                    else { auto bopt=find_book(isbn); if(!bopt){ print_invalid(); }
                        else { int q=stoi(qtys); if(bopt->stock<q){ print_invalid(); }
                            else { Book b=*bopt; b.stock-=q; double cost = b.price*q; if(!upsert_book(b)){ print_invalid(); } else { record_finance(cost, 0.0); cout<<fixed<<setprecision(2)<<cost<<"\n"; } }
                        }
                    }
                }
            }
        }
        else if(cmd=="select"){
            if(current_priv()<3){ print_invalid(); }
            else { vector<string> tok; string tmp=line; size_t pos=0; while(true){ size_t sp=tmp.find(' ', pos); if(sp==string::npos){ tok.push_back(tmp.substr(pos)); break; } tok.push_back(tmp.substr(pos, sp-pos)); pos=sp+1; }
                if(tok.size()!=2){ print_invalid(); }
                else { string isbn=tok[1]; if(!is_valid_isbn(isbn)){ print_invalid(); }
                    else { auto bopt=find_book(isbn); if(!bopt){ Book b{}; strcpy_safe(b.isbn, isbn); strcpy_safe(b.name, ""); strcpy_safe(b.author, ""); strcpy_safe(b.keyword, ""); b.price=0.0; b.stock=0; upsert_book(b); } login_stack.back().selected_isbn = isbn; }
                }
            }
        }
        else if(cmd=="modify"){
            if(current_priv()<3){ print_invalid(); }
            else if(login_stack.empty() || login_stack.back().selected_isbn.empty()){ print_invalid(); }
            else {
                string rest=line.substr(7); rest=normalize_spaces(rest); if(rest.empty()){ print_invalid(); }
                else {
                    map<string,string> kv; vector<string> parts; {
                        string cur; bool inq=false; for(size_t i=0;i<rest.size();++i){ char c=rest[i]; if(c=='"'){ inq=!inq; cur.push_back(c); } else if(c==' ' && !inq){ if(!cur.empty()){ parts.push_back(cur); cur.clear(); } } else cur.push_back(c);} if(!cur.empty()) parts.push_back(cur);
                    }
                    bool bad=false;
                    for(auto &t: parts){ if(t.size()<3 || t[0]!='-' || t.find('=')==string::npos){ print_invalid(); bad=true; break; } string key=t.substr(1, t.find('=')-1); string val=t.substr(t.find('=')+1); if(kv.count(key)){ print_invalid(); bad=true; break; } kv[key]=val; }
                    if(!bad){ auto bopt=find_book(login_stack.back().selected_isbn); if(!bopt){ print_invalid(); }
                        else { Book b=*bopt; bool ok=true; for(auto &p: kv){ string k=p.first, v=p.second; if(k=="ISBN"){ if(v.empty()||!is_valid_isbn(v)|| v==string(b.isbn)){ ok=false; break; } auto exists=find_book(v); if(exists){ ok=false; break; } strcpy_safe(b.isbn, v); login_stack.back().selected_isbn=v; }
                                else if(k=="name"){ if(v.size()<2||v.front()!='"'||v.back()!='"'){ ok=false; break; } string x=v.substr(1, v.size()-2); if(x.empty()||!is_valid_name_author(x)){ ok=false; break; } strcpy_safe(b.name, x); }
                                else if(k=="author"){ if(v.size()<2||v.front()!='"'||v.back()!='"'){ ok=false; break; } string x=v.substr(1, v.size()-2); if(x.empty()||!is_valid_name_author(x)){ ok=false; break; } strcpy_safe(b.author, x); }
                                else if(k=="keyword"){ if(v.size()<2||v.front()!='"'||v.back()!='"'){ ok=false; break; } string x=v.substr(1, v.size()-2); if(x.empty()||!is_valid_keyword(x)){ ok=false; break; } vector<string> segs; string cur; for(char c: x){ if(c=='|'){ if(cur.empty()){ ok=false; break; } segs.push_back(cur); cur.clear(); } else cur.push_back(c);} if(!ok) break; if(cur.empty()){ ok=false; break; } segs.push_back(cur); sort(segs.begin(), segs.end()); for(size_t i=1;i<segs.size();++i){ if(segs[i]==segs[i-1]){ ok=false; break; } } if(!ok) break; strcpy_safe(b.keyword, x); }
                                else if(k=="price"){ if(!is_valid_price_str(v)){ ok=false; break; } double pr; try{ pr=stod(v);}catch(...){ ok=false; break; } b.price=pr; }
                                else { ok=false; break; } }
                            if(!ok){ print_invalid(); } else { if(!upsert_book(b)) print_invalid(); }
                        }
                    }
                }
            }
        }
        else if(cmd=="import"){
            if(current_priv()<3){ print_invalid(); }
            else if(login_stack.empty() || login_stack.back().selected_isbn.empty()){ print_invalid(); }
            else { vector<string> tok; string tmp=line; size_t pos=0; while(true){ size_t sp=tmp.find(' ', pos); if(sp==string::npos){ tok.push_back(tmp.substr(pos)); break; } tok.push_back(tmp.substr(pos, sp-pos)); pos=sp+1; }
                if(tok.size()!=3){ print_invalid(); }
                else { string qtys=tok[1], totals=tok[2]; if(!is_positive_int(qtys)||!is_positive_number(totals)){ print_invalid(); }
                    else { int q=stoi(qtys); double total=stod(totals); auto bopt=find_book(login_stack.back().selected_isbn); if(!bopt){ print_invalid(); } else { Book b=*bopt; b.stock += q; upsert_book(b); record_finance(0.0, total); } }
                }
            }
        }
        else if(cmd=="report"){
            if(current_priv()<7){ print_invalid(); }
            else { if(line=="report finance"){ auto v=read_finance(); double inc=0, exp=0; for(auto &r:v){ inc+=r.income; exp+=r.expense; } cout<<"Finance Report\n"<<"Income: "<<fixed<<setprecision(2)<<inc<<"\n"<<"Expense: "<<fixed<<setprecision(2)<<exp<<"\n"; }
                else if(line=="report employee"){ cout<<"Employee Report\n"; fstream f(OPS_LOG, ios::in); string l; int cnt=0; if(f.good()) while(getline(f,l)){ if(l.find("su ")!=string::npos || l.find("logout")!=string::npos || l.find("select ")!=string::npos || l.find("modify ")!=string::npos || l.find("import ")!=string::npos){ cnt++; } } f.close(); cout<<"Operations: "<<cnt<<"\n"; }
                else { print_invalid(); }
            }
        }
        else if(cmd=="log"){
            if(current_priv()<7){ print_invalid(); }
            else { fstream f(OPS_LOG, ios::in); string l; if(f.good()){ while(getline(f,l)){ cout<<l<<"\n"; } } f.close(); }
        }
        else { print_invalid(); }

        log_op(line);
    }
    return 0;
}
