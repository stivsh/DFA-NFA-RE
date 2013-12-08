#ifndef POOLALLOC_H
#define POOLALLOC_H

#include <limits>
#include <vector>
using namespace std;
   template <class T>
   class PoolAllocator {
     public:
       typedef T        value_type;
       typedef T*       pointer;
       typedef const T* const_pointer;
       typedef T&       reference;
       typedef const T& const_reference;
       typedef std::size_t    size_type;
       typedef std::ptrdiff_t difference_type;
     private:
       vector<pointer> pages;
       vector<size_t> elements_in_pages;
       vector<pointer> free_adresses;
    public:
       static const size_t ElInPage=4*1024/sizeof(value_type);
       // rebind allocator to type U
       template <class U>
       struct rebind {
           typedef PoolAllocator<U> other;
       };
       int page_num(pointer p){
           for(int i=0;i!=pages.size();++i){
               if(pages[i]<=p && p<pages[i]+ElInPage)
                   return i;
           }
           return -1;
       }

       void alloc_new_page(){
           //alloc new page
           pointer new_page=new value_type[ElInPage];
           pages.push_back(new_page);
           for(pointer p=new_page;p!=new_page+ElInPage;++p){
            free_adresses.push_back(p);
           }
           elements_in_pages.push_back(0);
       }

       // return address of values
       pointer address (reference value) const {
           return &value;
       }
       const_pointer address (const_reference value) const {
           return &value;
       }

       /* constructors and destructor
        * - nothing to do because the allocator has no state
        */
       PoolAllocator(){
           free_adresses.reserve(ElInPage);
           alloc_new_page();
       }
       ~PoolAllocator(){
             for(size_t i=0;i!=pages.size();++i)
                 delete [] pages[i];
       }

       // return maximum number of elements that can be allocated
       size_type max_size () const throw() {
           return std::numeric_limits<std::size_t>::max() / sizeof(T);
       }

       // allocate but don't initialize num elements of type T
       pointer allocate (size_type num, const void* = 0) {
           if(num>1)
               throw 1;
           if(!free_adresses.size()){
            alloc_new_page();
           }

           pointer p=free_adresses.back();
           free_adresses.pop_back();
           elements_in_pages[page_num(p)]+=1;
           return p;
       }

       // initialize elements of allocated storage p with value value
       void construct (pointer p, const T& value) {
           // initialize memory with placement new
           new((void*)p)T(value);
       }

       // destroy elements of initialized storage p
       void destroy (pointer p) {
           // destroy objects by calling their destructor
           p->~T();
       }

       // deallocate storage p of deleted elements
       void deallocate (pointer p, size_type num) {
           (void)num;
           if(--elements_in_pages[page_num(p)]==0 && pages.size()>1){
               int _page_num=page_num(p);
               pointer page_addr=pages[_page_num];
               pages[_page_num]=pages.back();
               elements_in_pages[_page_num]=elements_in_pages.back();
               pages.pop_back();
               elements_in_pages.pop_back();
               delete[] page_addr;
           }else{
            free_adresses.push_back(p);
           }
       }
   };

   // return that all specializations of this allocator are interchangeable
   template <class T1, class T2>
   bool operator== (const PoolAllocator<T1>&,
                    const PoolAllocator<T2>&) throw() {
       return false;
   }
   template <class T1, class T2>
   bool operator!= (const PoolAllocator<T1>&,
                    const PoolAllocator<T2>&) throw() {
       return true;
   }
#endif // POOLALLOC_H
