import re
import os
from warcat import model
from datetime import datetime

def parsing(text_content):
	term_tf=dict()
	for m in re.finditer(r"[A-Za-z0-9]+'*[A-Za-z0-9]*", text_content):
		term, pos = m.group(0), (m.start(), m.end())
		# print(term, pos)
		if term in term_tf:
			term_tf[term]+=1
		else:
			term_tf[term]=1 # set as 0???
			
	return term_tf
	
def  output_file(term_tf, docID, f):
	for term, tf in term_tf.items():
		# print(term, tf)
		f.write(term+' '+str(docID)+' '+str(tf)+'\n')