from method_ import *
import os
import warcat.model
from datetime import datetime
start=datetime.now()
docID=0
# warc = warcat.model.WARC()
# print(type(warc))
# warc_file_obj=warc.open('0.gz') 
# print(datetime.now()-start)
# print(warcat.model.field.Fields())
os.system('sort b.txt /+1 /C[ASE_SENSITIVE]')
# record, is_end = warc.read_record(warc_file_obj)	# skip the first BlockWithPayload			
# while is_end:
	# record, is_end = warc.read_record(warc_file_obj)
	# text_content=record.content_block.get_file().read().decode()
	# print(record.header.fields.get('WARC-Target-URI'))
	# print(type(record.header.fields.get('Content-Length')))
	# print(type(record.header.fields.get('WARC-Target-URI')))
	# # for a in record.header:
		# # print(type(a),a)
	# if docID>=0:
		# break
		# # term_tf=parsing(text_content)
		# # output_file(term_tf, docID, f)
	# docID+=1
# print(datetime.now()-start)