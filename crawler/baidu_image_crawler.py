# coding:UTF-8
import time
import os
from bs4 import BeautifulSoup
import requests
import string
global path

def download(url, filename,fromHost):
    try:
        ir = requests.get(url)
        ir.raise_for_status()
        if ir.status_code == 200:
            filePathName = os.path.join(path, filename)
            open(filePathName, 'wb').write(ir.content)
        print "download %s suceese"%url
        return True
    except BaseException,e:
        print 'download error :%s'%filename
        print e.message
        return False

def request(params):
    headers = { "Accept":"text/html,application/xhtml+xml,application/xml;",
                "Accept-Encoding":"gzip",
                "Accept-Language":"zh-CN,zh;q=0.8",
                "Referer":"http://http://www.baidu.com/",
                "User-Agent":"Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/42.0.2311.90 Safari/537.36"
                }

    def decode_url(url):
        in_table =  u'0123456789abcdefghijklmnopqrstuvw'
        out_table = u'7dgjmoru140852vsnkheb963wtqplifca'
        translate_table = string.maketrans(in_table, out_table)
        mapping = {'_z2C$q': ':', '_z&e3B': '.', 'AzdH3F': '/'}
        for k, v in mapping.items():
            url = url.replace(k, v)
        url = url.encode()
        return url.translate(translate_table)

    try:
        url = "http://image.baidu.com/search/acjson"
        response = requests.get(url, params=params,headers=headers)
        response.raise_for_status()
        response.encoding = response.apparent_encoding
        jsons = response.json()['data']
        for json in jsons:
            image_urls = []
            if 'objURL' in json.keys():
                image_urls.append(decode_url(json['objURL']))
            if 'replaceUrl' in json.keys() and len(json['replaceUrl']) == 2:
                image_urls.append(json['replaceUrl'][1]['ObjURL'])
            

            print len(image_urls)
            for objUrl in image_urls:
                filename = os.path.split(objUrl)[1].split('?')[0]
                if(len(filename) != 0 and filename.find('.') >= 0):
                    fromHost = json['fromURLHost']
                    print 'Downloading from %s' % objUrl
                    if(download(objUrl, filename,fromHost)):
                       break 


    except BaseException,e:
        print e.message
        return "get url error"

def search(keyword, minpage, maxpage):
    params = {
        'tn': 'resultjson_com',
        'word': keyword,
        'queryWord':keyword,
        'ie': 'utf-8',
        'cg': '',
        'ct':'201326592',
        'fp':'result',
        'cl':'2',
        'lm':'-1',
        'rn': '30',
        'ipn':'rj'
    };
    for i in range(minpage, maxpage):
        print 'Download page %d:'%i 
        params['pn'] = '%d' % (i * 30)
        request(params)
    print 'download end'

def start(keyword,startpage,endpage,inpath=''):
    if len(inpath) == 0:
        inpath = os.curdir + '/'+keyword
    global path
    path = inpath.decode('utf-8')
    print 'download image to %s'%path
    if os.path.exists(path) == False:
        os.mkdir(path)

    search(keyword, startpage, endpage)
