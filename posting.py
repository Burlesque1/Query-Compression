from method import *
from datetime import datetime


num = 0
docID = 0
count = 0
binary_flag = False # True for ascii 
dir_tag = False
start_time = datetime.now()
print(str(start_time))
	
for root, dirs, files in os.walk('H:\\NYU\\CS 6913\\Assignment2\\NZ'):
	for fname in files:
		if '.tar' in fname:
			print(fname)
			tar_f = os.path.join(root,fname)
			# print(tar_f)
			docID = handle_tar_file(tar_f, docID)
			print('\n' + fname + ' complete\n')			
			print(int((datetime.now()-start_time).seconds), 's' , docID)
			

finish_time = datetime.now()
print('The running time is ', str((finish_time-start_time).seconds), 's')
print('Total page amount is ', docID)
print('Speed is ',docID/int((finish_time-start_time).seconds), '/s')
	
	# # for NZ2!
# data_f = "G:\\nz2_merged\\" + str(num) + "_data"
# index_f = "G:\\nz2_merged\\" + str(num) + "_index"	
# while num < 83:
	# if num%500 == 0:
		# dir_tag = True # every 500 pages within a folder
	# else:
		# dir_tag = False	
		# num += 1
	# print(data_f + ' complete\n', docID, ' docID')
	

# for root, dirs, files in os.walk('G:\\nz2_merged'):
	# for name in files:
		# input('d')
		# if '_data' in name:
			# data_f = os.path.join(root,name)
		# if  '_index' in name:
			# index_f = os.path.join(root,name)
			# docID = handle_index_file(index_f, docID, data_f, dir_tag)
			# print(name, docID, int((datetime.now()-start_time).seconds))