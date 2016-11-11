					# /* -----------------add this information------------------------------
					# MetaDataSize TotalBlocks LastDocId1 .. LastDocIdn SizeOfBlock1 .. SizeOfBlock2 DocId1 DocId2 .. DocIdn Freq1 .. Freqn
	 
					# Taking example from your last answer
					# Actual docIDs:      [ 2  4  7  9 ]  [ 13  15  21  23 ]  [ 28  31  36  43 ] 
					
					# Then we store like below
					# 3 9 23 43 4 4 4 2 2 3 9 4 2 6 23 5 3 5 43 ...(frequencies afterwards)
					# ---------------------------------------------------------------------*/i=0
i=0
ff=open('lexicon.bin', 'rb+')
with open('lexicon.bin', 'rb') as f:
	print(f.read())
ff.close()