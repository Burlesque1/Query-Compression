import tarfile
import gzip
import zlib
import re
import os



def word_parsing_tool(data_block, w_file, docID):
	patternTag = "<[^>]*>"
	patternBlank = r"[\t\r\n\f\v]";
	reg1 = re.compile(patternTag)
	reg2 = re.compile(patternBlank)
	reg3 = re.compile(r'&nbsp;')
	content1 = reg1.sub(' ', data_block)	# remove html tags and some irrelevant chars
	content2 = reg2.sub(' ', content1)		
	content3 = reg3.sub(' ', content2)
		
	for m in re.finditer(r"[A-Za-z0-9]+", content3):
		term, pos = m.group(0), (m.start(), m.end())
		w_file.write(term + ' ' + str(docID)  + '\n')		
	

	 
	 
	 
def create_docID_table(url, docID, page_table):
	page_table.write(str(url)+ " " + str(docID) + "\n")
	pass

	
# return bytes stream
def decompress(tar, target):
	member = tar.getmember(target)
	de_tar = tar.extractfile(member).read()
	de_gz= zlib.decompress(de_tar, zlib.MAX_WBITS|32)	
	return de_gz
	
	
# find corresponding data in _data file and pass it to word parser
def handle_data(docID, degz_index, degz_data, page_table, w_file):
	pos_accum = 0
	for line in degz_index.decode(encoding='iso-8859-1').replace('b\'', '').split('\n')[0:-1]:	
		# print(line)
		line = line.split(' ')
		url = line[0]
		size = int(line[3])
		
		# read block from degz_data
		data_block = degz_data[pos_accum:(pos_accum + size)].decode(encoding='iso-8859-1')	
		sp = data_block.find('<')
		
		# Skip error pages and truncated pages
		if int(data_block[9:13])>=400 or ('</html>' not in data_block[-20:]): 
			continue
			
		# create page (or docID-to-URL) table.
		try:
			s_line = url + " " + str(docID) + "\n"
			page_table.write(s_line.encode())	# create_docID_table(url, docID, page_table)		
		except Exception as e:
			print(e)
			print(url.encode())
	
		# call parser to parse decomp_data and generate initial posting
		word_parsing_tool(data_block[sp:], w_file, docID)
		pos_accum += size
		docID += 1
	return docID	
	# end of a n_index file
	
	
	
# decompress tar file and pass gz file to next method
def handle_tar_file(tar_f, docID):
	directory  = "posting/" 
	if not os.path.exists(directory):
		print('Creating directory ' + directory)
		os.makedirs(directory)	
	with  tarfile.open(tar_f, "r") as tar:
		w_file = open(directory + tar.getnames()[0][40:], 'a')				# remember to set as binary/ascii
		page_table = open('page_table','ab')		# add exception
		for data_mname in tar.getnames():		# 100 files
			if '2406' in data_mname:
				print('skip')
				continue
			if '_data' in data_mname:
				pos = data_mname.find('_data')
				index_mname = data_mname[0:pos] + '_index'
				
				# decomp index
				degz_index = decompress(tar, index_mname)
				
				# decomp data
				degz_data = decompress(tar, data_mname)
				
				# parse url and generate url-table, intermediate postings
				docID = handle_data(docID, degz_index, degz_data, page_table, w_file)
				print('\n' + data_mname + ' complete\n', docID, ' docID')
		w_file.close()
		page_table.close()
	return docID
	
	
	
	
	
	# for nz2	
def handle_data_file(pos, page_size, ff):
	decomp_data= ff.read(page_size)
	# print(decomp_data[0:100])
	return decomp_data
	# gf = gzip.GzipFile(file_name)
	# print(gf.peek(page_size))
	
	
	
def handle_index_file(file_name, docID, data_f, dir_tag):
	pos_accum = 0
	count = docID
	
	directory = "posting_" 
	posting = '/' + file_name[-7]
	if not os.path.exists(directory):
		print('Creating directory ' + directory)
		os.makedirs(directory)
		
	# generate intermediate posting
	w_file = open(directory + posting, 'a')				# remember to set as binary/ascii
	page_table = open('page_table_nz2','a')		# add exception
	# with gzip.open(file_name) as f:			# reduce open/close num
		# with gzip.open(data_f, 'rb') as ff:		# reduce time complexity	
			# for line in f:
				# line = str(line).split(' ')
				# url = line[0]
				# size = int(line[3])
				
				# create page (or docID-to-URL) table.
				# create_docID_table(url, docID, page_table)
				
				# # uncompressing gzip file	
				# decomp_data = handle_data_file(pos_accum, size, ff).decode(encoding='iso-8859-1')		
					
				# # call parser to parse decomp_data and generate initial posting
				# word_parsing_tool(decomp_data, w_file, docID)
				
				# pos_accum += size
				# docID += 1
				# if  docID - count > 5:
					# break
	f = 	gzip.open(file_name)
	ff = gzip.open(data_f, 'rb')
	for line in f:
		line = str(line).split(' ')
		url = line[0]
		size = int(line[3])
			
		# # create page (or docID-to-URL) table.
		create_docID_table(url, docID, page_table)
		
		# uncompressing gzip file	
		decomp_data = handle_data_file(pos_accum, size, ff).decode(encoding='iso-8859-1')		
					
		# call parser to parse decomp_data and generate initial posting
		word_parsing_tool(decomp_data, w_file, docID)
			
		pos_accum += size
		docID += 1
	f.close()
	ff.close()
	w_file.close()
	page_table.close()
	return docID