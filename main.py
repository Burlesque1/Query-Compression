from method_ import *

start = datetime.now()
print(start)
i=0
docID=0
warc = model.WARC()
url_table=open('URL-TABLE','w')
for root, dirs, files in os.walk('H:\\NYU\\CS 6913\\Assignment3\\data'):
	# print(root, dirs, files)
	for fname in files:
		if '0.gz' == fname: # '.gz' in fname:
			print(fname)
			warc_file_obj=warc.open(fname) 
			record, is_end = warc.read_record(warc_file_obj)	# skip the first BlockWithPayload			
			file_name=str(i)+'_posting'
			with open(file_name, "a") as f:
				while is_end:
					record, is_end = warc.read_record(warc_file_obj)
					
					text_content=record.content_block.get_file().read().decode()
					url=record.header.fields.get('WARC-Target-URI')
					length=record.header.fields.get('Content-Length')
					
					url_table.write(str(docID)+' '+url+' '+length+'\n')
					
					# term_tf=parsing(text_content)
					# output_file(term_tf, docID, f)
					
					docID+=1
					# if docID>=10000:
						# break
					if docID%100==0:
						print(docID, datetime.now()-start)
			print(docID, datetime.now()-start)
			# os.system('sort '+ file_name+' /C /rec 65535 /O '+ file_name+'_sorted')
			i+=1
		# if i>0:
			# break
url_table.close()
print(docID, (datetime.now()-start).seconds)
