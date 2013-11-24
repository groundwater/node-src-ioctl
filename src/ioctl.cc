#include <iostream>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/route.h>
#include <v8.h>
#include <node.h>
#include <unistd.h>
#include <cstring>

// http://man7.org/linux/man-pages/man7/netdevice.7.html
// http://linux.about.com/library/cmd/blcmdl7_netdevice.htm

using namespace std;
using namespace v8;
using namespace node;

namespace NodeOS
{
  
  static Handle<Object> GetFlags(){
    HandleScope scope;
    Local<Object> flags = Object::New();
    
    flags->Set( String::New("SIOCGIFNAME"), Integer::New(SIOCGIFNAME) );
    flags->Set( String::New("SIOCGIFINDEX"), Integer::New(SIOCGIFINDEX) );
    flags->Set( String::New("SIOCGIFFLAGS"), Integer::New(SIOCGIFFLAGS) );
    flags->Set( String::New("SIOCSIFFLAGS"), Integer::New(SIOCSIFFLAGS) );
    flags->Set( String::New("SIOCGIFPFLAGS"), Integer::New(SIOCGIFPFLAGS) );
    flags->Set( String::New("SIOCSIFPFLAGS"), Integer::New(SIOCSIFPFLAGS) );
    flags->Set( String::New("SIOCGIFADDR"), Integer::New(SIOCGIFADDR) );
    flags->Set( String::New("SIOCSIFADDR"), Integer::New(SIOCSIFADDR) );
    flags->Set( String::New("SIOCGIFDSTADDR"), Integer::New(SIOCGIFDSTADDR) );
    flags->Set( String::New("SIOCSIFDSTADDR"), Integer::New(SIOCSIFDSTADDR) );
    flags->Set( String::New("SIOCGIFBRDADDR"), Integer::New(SIOCGIFBRDADDR) );
    flags->Set( String::New("SIOCSIFBRDADDR"), Integer::New(SIOCSIFBRDADDR) );
    flags->Set( String::New("SIOCGIFNETMASK"), Integer::New(SIOCGIFNETMASK) );
    flags->Set( String::New("SIOCSIFNETMASK"), Integer::New(SIOCSIFNETMASK) );
    flags->Set( String::New("SIOCGIFMETRIC"), Integer::New(SIOCGIFMETRIC) );
    flags->Set( String::New("SIOCSIFMETRIC"), Integer::New(SIOCSIFMETRIC) );
    flags->Set( String::New("SIOCGIFMTU"), Integer::New(SIOCGIFMTU) );
    flags->Set( String::New("SIOCSIFMTU"), Integer::New(SIOCSIFMTU) );
    flags->Set( String::New("SIOCGIFHWADDR"), Integer::New(SIOCGIFHWADDR) );
    flags->Set( String::New("SIOCSIFHWADDR"), Integer::New(SIOCSIFHWADDR) );
    flags->Set( String::New("SIOCSIFHWBROADCAST"), Integer::New(SIOCSIFHWBROADCAST) );
    flags->Set( String::New("SIOCGIFMAP"), Integer::New(SIOCGIFMAP) );
    flags->Set( String::New("SIOCSIFMAP"), Integer::New(SIOCSIFMAP) );
    flags->Set( String::New("SIOCADDMULTI"), Integer::New(SIOCADDMULTI) );
    flags->Set( String::New("SIOCGIFTXQLEN"), Integer::New(SIOCGIFTXQLEN) );
    flags->Set( String::New("SIOCSIFTXQLEN"), Integer::New(SIOCSIFTXQLEN) );
    flags->Set( String::New("SIOCSIFNAME"), Integer::New(SIOCSIFNAME) );
    flags->Set( String::New("SIOCGIFCONF"), Integer::New(SIOCGIFCONF) );
    
    return scope.Close(flags);
  }
  
  void deleteWrapped(Persistent<Value> object, void *parameter){
    Persistent<External> ext = Persistent<External>::Cast(object);
    delete External::Unwrap(ext);
  }
  
  const char * HWADDR_FMT = "%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X";
  
  Handle<Value> NewIFReqSocketaddrIn( Handle<Object> obj ){
    HandleScope scope;
    sockaddr_in sock;
    
    String::Utf8Value addr(Handle<String>::Cast(obj->Get(String::New("sin_addr"))));
    
    int port = Handle<Integer>::Cast(obj->Get(String::New("sin_port")))->Value();
    int family = Handle<Integer>::Cast(obj->Get(String::New("sin_family")))->Value();
    
    int res = inet_aton(*addr, &sock.sin_addr);
    
    sock.sin_port = port;
    sock.sin_family = family;
    
    Handle<Value> ext = External::Wrap(&sock);
    
    return scope.Close(ext);
  }
  
  Handle<Object> GetIFReqSocket(const Arguments& args){
    HandleScope scope;
    
    Handle<Object> obj = Object::New();
    
    Handle<External> ext = Handle<External>::Cast(args[0]);
    sockaddr *sock = (sockaddr*) External::Unwrap(ext);
    
    char* hwaddr = sock->sa_data;
    char out [18];
    sprintf(out,HWADDR_FMT,hwaddr[0],hwaddr[1],hwaddr[2],hwaddr[3],hwaddr[4],hwaddr[5]);
    
    obj->Set(String::NewSymbol("sa_data"), String::New(out));
    obj->Set(String::NewSymbol("sa_family"), Integer::New(sock->sa_family));
    return scope.Close(obj);
  }
  
  Handle<Object> GetIFReqSocketIn(const Arguments& args){
    HandleScope scope;
    Handle<Object> obj = Object::New();
    
    Handle<External> ext = Handle<External>::Cast(args[0]);
    sockaddr_in *sock_in = (sockaddr_in*) External::Unwrap(ext);
    
    obj->Set(String::NewSymbol("sin_family"), Integer::New(sock_in->sin_family));
    obj->Set(String::NewSymbol("sin_port"), Integer::New(sock_in->sin_port));
    
    char* ip = inet_ntoa(sock_in->sin_addr);
    
    obj->Set(String::NewSymbol("sin_addr"), String::New(ip));
    return scope.Close(obj);
  }
  
  // static Handle<Value> Decode(const Arguments& args){
  //   HandleScope scope;
  //   
  //   Handle<External> ext = Handle<External>::Cast(args[0]);
  //   
  //   String::Utf8Value type_( args[1] );
  //   char *type = *type_;
  //   
  //   ifreq* req = (ifreq*) External::Unwrap(ext);
  //   
  //   Handle<Object> obj = Object::New();
  //   
  //   char* name = new char[ sizeof(req->ifr_name) ];
  //   
  //   strcpy(name,req->ifr_name);
  //   
  //   obj->Set(String::NewSymbol("name"), String::New(name));
  //   
  //   if(strcmp(type,"addr")==0){
  //     obj->Set(String::NewSymbol("addr"), GetIFReqSocketIn(&req->ifr_addr));
  //   }else if(strcmp(type,"hwaddr")==0){
  //     obj->Set(String::NewSymbol("hwaddr"), GetIFReqSocket(&req->ifr_addr));
  //   }else if(strcmp(type,"number")==0){
  //     obj->Set(String::NewSymbol("number"), Integer::New(req->ifr_ifindex));
  //   }
  //   
  //   return scope.Close(obj);
  // }
  
  static Handle<Value> GetIFReqName(const Arguments& args){
    HandleScope scope;
    
    Handle<External> ext = Handle<External>::Cast(args[0]);
    
    String::Utf8Value type_( args[1] );
    char *type = *type_;
    
    ifreq* req = (ifreq*) External::Unwrap(ext);
    Handle<String> name = String::New( req->ifr_name );
    
    return scope.Close(name);
  }
  
  static Handle<Value> Socket(const Arguments& args){
    HandleScope scope;
    
    int fd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);
    
    return scope.Close(Integer::New(fd));
  }
  
  static Handle<Value> IFReq(const Arguments& args){
    HandleScope scope;
    
    ifreq *req = new ifreq;
    
    if(args.Length()==1){
      String::Utf8Value name(args[0]);
      strcpy(req->ifr_name, *name);
    }
    
    Handle<Value> obj = External::Wrap(req);
    Persistent<Value> ext = Persistent<Value>::New(obj);
    ext.MakeWeak(NULL,deleteWrapped);
    return scope.Close(obj);
  }
  
  static Handle<Value> Ioctl(const Arguments& args){
    HandleScope scope;
    
    int sfd = Handle<Number>::Cast(args[0])->Value();
    int com = Handle<Number>::Cast(args[1])->Value();
    Handle<External> obj = Handle<External>::Cast( args[2] );
    
    ifreq *req = (ifreq*) External::Unwrap(obj);
    
    int res = ioctl( sfd, com, req );
    
    return scope.Close(Integer::New(res));
  }
   
  static void init(Handle<Object> target) {
    
    target->Set(String::NewSymbol("flags"), GetFlags() );
    
    target->Set(String::NewSymbol("socket"),
      FunctionTemplate::New(Socket)->GetFunction());
    
    target->Set(String::NewSymbol("getIFReqName"),
      FunctionTemplate::New(GetIFReqName)->GetFunction());
  
    // target->Set(String::NewSymbol("decode"),
    //   FunctionTemplate::New(Decode)->GetFunction());
      
    target->Set(String::NewSymbol("ifreq"),
      FunctionTemplate::New(IFReq)->GetFunction());
    
    target->Set(String::NewSymbol("ioctl"),
      FunctionTemplate::New(Ioctl)->GetFunction());
    
  }
}

NODE_MODULE(binding, NodeOS::init)
