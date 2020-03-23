#ifndef __REDISAPP_H__
#define __REDISAPP_H__
#include <string>
#include <map>
#include <list>
#include <hiredis/hiredis.h>

using namespace std;

/* example :
		I'm using following command to save my data

		HMSET myhash ordertype "neworder" order_ts "1234" act_type "order_ack" ack_ts "1240"
		HMSET myhash2 ordertype "neworder" order_ts "2234" act_type "order_ack" ack_ts "2240"

		SADD hashkeys "myhash" "myhash1" "myhash2"

		SMEMBERS hashkeys
		1) "myhash"
		2) "myhash2"
		3) "myhash1"

		If you are using 2.8 I would recommend you use sscan instead of smembers

		sscan hashkeys 0 match my*

		USE ONLY SCAN COMMAND

		scan 0 MATCH myhash* count 1000 

		USE key matching (NOT RECOMMENDED AS it will block the redis server)

		keys myhash*

*/
typedef map<string*,string*> redisStoreInfo;

class RedisApp{

	RedisApp();

public:

	~RedisApp();

	static RedisApp &GetInstance();
	
	int RedisPushInfo(const string key,redisStoreInfo &info);
	
	int RedisDelInfo(const string key);

	int RedisGetAllStoreInfo(list<redisStoreInfo *> &infoList);

	int RedisClrAllStoreInfo();
private:
	int GetAllKyes(list<string> &keys);
	int GetAllHFields(string key,list<string> &fieldsList);
	int GetInfoUseKye(const string key,redisStoreInfo &info);
	
	redisContext *m_context;
	string m_redisServerIp;
	int m_redisServerPort;
};

#endif

