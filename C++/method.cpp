#include "method.h"

int read_b_file(string file_name)	// used for checking final answer
{
  streampos size;
  int * memblock;
//
//  ifstream file ("inverted-index.bin", ios::in|ios::binary|ios::ate);  // ios::ate	set the initial position at the end of the file.
//  															// otherwise the initial position is the beginning of the file.
//  if (file.is_open())
//  {
//    size = file.tellg();		// size of the whole file
//    memblock = new int [size];
//    file.seekg (0, ios::beg);	// remember that we opened the file with this pointer at the end
//    file.read (memblock, size);
//    file.close();
//
//    cout << "the entire file content is in memory\n";
//	for(int i=0;i*4<size;i++)
//		cout << memblock[i] << " "<<i<<endl;
//    delete[] memblock;
//    return 1;	// succeed return 1
//  }
//  else 
//  {
//	cout << "Unable to open file";
//	return -1;	// fail return -1
//  }
}

int foo(int a)
{
	cout<<"dfdf "<<a<<endl;
}
