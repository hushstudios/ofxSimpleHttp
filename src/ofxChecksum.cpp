
#include "ofxChecksum.h"

#include <Poco/SHA1Engine.h>
#include <Poco/DigestStream.h>

#include "../libs/xxHash/xxhash.h"

//#include "ofxTimeMeasurements.h"
//extern "C" {
//#include "sha1.h"
//}


bool ofxChecksum::sha1(const std::string& filePath,
					   const std::string& sha1String,
					   bool verbose){
	float t;
	if(verbose){
 		t = ofGetElapsedTimef();
	}

	Poco::SHA1Engine sha1e;
	std::string localHash = calcSha1(filePath);
	bool match = sha1String.compare(localHash) == 0;

 	if(verbose){
		t = ofGetElapsedTimef() - t;
		ofLogNotice("ofxChecksum") << "ofxChecksum::sha1(" << localHash << ") took " << t << " secs to calc.";
	}
	return match;
}


std::string ofxChecksum::calcSha1(const std::string & filePath){

//	TS_START_NIF("sha1_1");
	Poco::SHA1Engine e;
	{
		auto f = fopen(filePath.c_str(), "rb");
		if(f == NULL){
			ofLogError("ofxChecksum") << "can't calcSha1(); can't open file at \"" << filePath << "\"";
			return "";
		}
		vector<char> buf(1024 * 1024);
		size_t bytes_read = 0;
		do {
			bytes_read = fread(buf.data(), 1, buf.size(), f);
			if (ferror(f)) ofLogError("ofxChecksum") << "Error reading " << filePath << " for SHA calculation.";
			e.update(buf.data(), bytes_read);
		} while (bytes_read == buf.size());
		fclose(f);
	}
	return Poco::DigestEngine::digestToHex(e.digest());

//	TS_STOP_NIF("sha1_1");

	///////////////

	/*
	TS_START_NIF("sha1_2");
	blk_SHA_CTX sha1State;
	blk_SHA1_Init(&sha1State);
	auto f = fopen(filePath.c_str(), "rb");


	if(f == NULL){
		ofLogError("ofxChecksum") << "can't calcSha1(); can't open file at \"" << filePath << "\"";
		return "";
	}
	vector<char> buf( 10 * 1024 * 1024);
	size_t bytes_read = 0;
	do {
		bytes_read = fread(buf.data(), 1, buf.size(), f);
		if (ferror(f)) ofLogError("ofxChecksum") << "Error reading " << filePath << " for SHA calculation.";
		blk_SHA1_Update(&sha1State, buf.data(), bytes_read);
	} while (bytes_read == buf.size());

	unsigned char bufferHash[20];
	blk_SHA1_Final(bufferHash, &sha1State);


	char * bufferHash2 = sha1_to_hex(bufferHash);
	string sha12 = bufferHash2;


	TS_STOP_NIF("sha1_2");
	///////////////

	return sha12;
	 */
}


std::string ofxChecksum::calcSha1FromString(const std::string & data){
	Poco::SHA1Engine e;
	e.update(data);
	return Poco::DigestEngine::digestToHex(e.digest());
}


std::string ofxChecksum::xxHash(const std::string & filePath) {

	size_t const blockSize = 64 * 1024;
	FILE * f = fopen( filePath.c_str(), "rb" );
	if(f == NULL){
		ofLogError("ofxChecksum") << "can't xxHash(); can't open file at \"" << filePath << "\"";
		return 0;
	}

	int seed = 0;
	vector<char> buf(1024 * 1024);

	XXH64_state_t* const state = XXH64_createState();

	XXH_errorcode const resetResult = XXH64_reset(state, seed);
	if (resetResult == XXH_ERROR) abort();

	size_t bytes_read = 0;
	do {
		bytes_read = fread(buf.data(), 1, buf.size(), f);
		if (ferror(f)) ofLogError("ofxChecksum") << "Error reading " << filePath << " for xxHash calculation.";
		XXH_errorcode const addResult = XXH64_update(state, buf.data(), bytes_read);
		if (addResult == XXH_ERROR) abort();
	} while (bytes_read == buf.size());

	unsigned long long const hash = XXH64_digest(state);

	XXH64_freeState(state);
	fclose(f);

	//convert long long to hex string
	char buff[128];
	sprintf(buff, "%llx", hash);
	return string(buff);
}


