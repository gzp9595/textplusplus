from flask import Flask,json,request,abort,url_for
import time,random,string
from app import app
from config import *
import socket
import MySQLdb

def check_token(token):

	conn=MySQLdb.connect(host='127.0.0.1',user=dbuser,passwd=dbpw,port=3306)
	conn.select_db('thunlp')
	cursor=conn.cursor()
	cursor.execute('select * from user where token = %s', (token,))
	u = cursor.fetchone()
	conn.close()
	if u is None:
		return False
	return True

@app.route('/api/semlac', methods=['GET','POST'])
def semlac():
	if not request.form:
		return json.dumps({ 'code': 201,'message': 'format error' }), 400
	
	result = []	
	raws = request.form
	token = raws['token']
	
	print raws
	if(check_token(token) == False):
		return json.dumps({'code': 205,'message': 'the token is invalid'}), 203

	raw = raws['text'].encode("utf-8")

	client = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
	client.connect("/tmp/lac.sock")
	
	client.send(raw+"\0\0")
	raw1 = client.recv(65536)
	client.close()

	raw2 = raw1.strip().split(" ")

	ans = map(lambda x:{"word":x.split("_")[0], "type":x.split("_")[1]}, raw2)

	return json.dumps({'code': 100,'message': 'success','result': ans})

@app.route('/api/semctc', methods=['GET','POST'])
def semctc():
	if not request.form:
		return json.dumps({ 'code': 201,'message': 'format error' }), 400
	
	result = []	
	raws = request.form
	token = raws['token']
	
	# print raws
	if(check_token(token) == False):
		return json.dumps({'code': 205,'message': 'the token is invalid'}), 203

	raw = raws['text'].encode("utf-8")

	client = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
	client.connect("/tmp/ctc.sock")
	
	client.send(raw+"\0\0")
	raw1 = client.recv(65536)
	client.close()
	b = raw1.split('\n')
	result = []
	for i in b:
		c = i.split('\t')
		if(len(c) > 1):
			ans = {}
			ans['classification'] = c[0]
			ans['possibility'] = c[1]
			result.append(ans)
	print result
	return json.dumps({'code': 100,'message': 'success','classify_data': result})

@app.route('/api/semcke', methods=['GET','POST'])
def semcke():
	if not request.form:
		return json.dumps({ 'code': 201,'message': 'format error' }), 400
	
	result = []	
	raws = request.form
	token = raws['token']
	
	# print raws
	if(check_token(token) == False):
		return json.dumps({'code': 205,'message': 'the token is invalid'}), 203

	raw = raws['text'].encode("utf-8")

	client = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
	client.connect("/tmp/ctc.sock")
	
	client.send(raw+"\0\0")
	wordlist = []
	raw1 = client.recv(65536)
	client.close()
	b = raw1.split('\n')

	for i in b:
		if(i == ''):
			break
		c = i.split('\t')
		word = {}
		word['text'] = c[0]
		word['weight'] = c[1]
		wordlist.append(word)

	return json.dumps({'code': 100,'message': 'success','word_list': wordlist})


@app.route('/api/userinfo', methods=['GET','POST'])
def getuserinfo():
	pass