#include "thulac_base.h"
#include "preprocess.h"
#include "postprocess.h"
#include "punctuation.h"
#include "cb_tagging_decoder.h"
#include "chinese_charset.h"
#include "filter.h"
#include "timeword.h"
#include "verbword.h"
#include <sstream>
#include <fstream>
using namespace thulac;

bool checkfile(const char* filename){
	std::fstream infile;
	infile.open(filename, std::ios::in);
	if(!infile){
		return false;
	}else{
		infile.close();
		return true;
	}
}

void showhelp(){
	std::cerr<<"Command line usage:"<<std::endl;
	std::cerr<<"./thulacpos [-t2s] [-filter] [-deli delimeter] [-user userword.txt] [-model_dir dir]"<<std::endl;
	std::cerr<<"or"<<std::endl;
	std::cerr<<"./thulacpos [-t2s] [-filter] [-deli delimeter] [-user userword.txt] <inputfile >outputfile"<<std::endl;
	std::cerr<<"\t-t2s\t\t\ttransfer traditional Chinese text to Simplifed Chinese text"<<std::endl;
	std::cerr<<"\t-filter\t\t\tuse filter to remove the words that have no much sense, like \"could\""<<std::endl;
	std::cerr<<"\t-deli delimeter\t\tagsign delimeter between words and POS tags. Default is _"<<std::endl;
	std::cerr<<"\t-user userword.txt\tUse the words in the userword.txt as a dictionary and the words will labled as \"uw\""<<std::endl;
	std::cerr<<"\t-model_dir dir\t\tdir is the directory that containts all the model file. Default is \"models/\""<<std::endl;
}

int main (int argc,char **argv) {

	char* user_specified_dict_name=NULL;
	char* model_path_char = NULL;


	Character separator = '_';

	bool useT2S = false;
	bool useFilter = false;

	int c = 1;
	while(c < argc){
		std::string arg = argv[c];
		if(arg == "-t2s"){
			useT2S = true;
		}else if(arg == "-user"){
			user_specified_dict_name = argv[++c];
		}else if(arg == "-deli"){
			separator = argv[++c][0];
		}else if(arg == "-filter"){
			useFilter = true;
		}else if(arg == "-model_dir"){
			model_path_char = argv[++c];
		}else{
			showhelp();
			return 1;
		}
		c ++;
	}

	std::string prefix;
	if(model_path_char != NULL){
		prefix = model_path_char;
		if(*prefix.rbegin() != '/'){
			prefix += "/";
		}
	}else{
		prefix = "models/";
	}

	TaggingDecoder* tagging_decoder = NULL;
	permm::Model* tagging_model = NULL;
	DAT* tagging_dat = NULL;
	char** tagging_label_info = NULL;
	int** tagging_pocs_to_tags = NULL;
	tagging_decoder = new TaggingDecoder();
	tagging_decoder->separator = separator;
	tagging_decoder->threshold = 0;
	tagging_model = new permm::Model((prefix+"model_c_model.bin").c_str());
	tagging_dat = new DAT((prefix+"model_c_dat.bin").c_str());
	tagging_label_info = new char*[tagging_model->l_size];
	tagging_pocs_to_tags = new int*[16];
	
	get_label_info((prefix+"model_c_label.txt").c_str(), tagging_label_info, tagging_pocs_to_tags);
	tagging_decoder->init(tagging_model, tagging_dat, tagging_label_info, tagging_pocs_to_tags);
	tagging_decoder->set_label_trans();
	
	Preprocesser* preprocesser = new Preprocesser();
	preprocesser->setT2SMap((prefix+"t2s.dat").c_str());

	Postprocesser* ns_dict = new Postprocesser((prefix+"ns.dat").c_str(), "ns", false);
	Postprocesser* idiom_dict = new Postprocesser((prefix+"idiom.dat").c_str(), "i", false);
	Postprocesser* nz_dict = new Postprocesser((prefix+"nz.txt").c_str(), "nz", true);
	Postprocesser* noun_dict = new Postprocesser((prefix+"noun.txt").c_str(), "n", true);
	Postprocesser* vm_dict = new Postprocesser((prefix+"vm.txt").c_str(), "vm", true);
	Postprocesser* y_dict = new Postprocesser((prefix+"y.txt").c_str(), "y", true);

	Postprocesser* user_dict = NULL;
	if(user_specified_dict_name){
		user_dict = new Postprocesser(user_specified_dict_name, "uw", true);
	}

	Punctuation* punctuation = new Punctuation((prefix+"singlepun.dat").c_str());

	TimeWord* timeword = new TimeWord();
	VerbWord* verbword = new VerbWord((prefix+"vM.dat").c_str(), (prefix+"vD.dat").c_str());

	Filter* filter = NULL;
	if(useFilter){
		filter = new Filter((prefix+"xu.dat").c_str(), (prefix+"time.dat").c_str());
	}

	POCGraph poc_cands;
	int rtn=1;
	thulac::RawSentence oriRaw;
	thulac::RawSentence raw;
	thulac::SegmentedSentence segged;
	thulac::TaggedSentence tagged;

	const int BYTES_LEN=10000;
	char* s=new char[ BYTES_LEN];
	char* out=new char[BYTES_LEN];
	std::string ori;

	bool isFirstLine = true;
	int codetype = -1;

	Chinese_Charset_Conv conv;

	std::ostringstream ost;

	clock_t start = clock();

	while(std::getline(std::cin,ori)){

		if(ori.length()>9999){
			ori = ori.substr(0,9999);
		}
		strcpy(s,ori.c_str());
		size_t in_left=ori.length();

		if(isFirstLine){ 
			size_t out_left=BYTES_LEN;
			codetype = conv.conv(s,in_left,out,out_left);
			if(codetype >=0){
				int outlen=BYTES_LEN - out_left;
				//thulac::get_raw(oriRaw,out,outlen);
				//std::cout<<"Here"<<std::endl;
				thulac::get_raw(oriRaw,out,outlen);
			}else{
				std::cout<<"File should be encoded in UTF8 or GBK."<<"\n";
				break;
			}
			isFirstLine = false;
		}else{
			if(codetype == 0){
				//thulac::get_raw(oriRaw,s,in_left);
				thulac::get_raw(oriRaw,s,in_left);
			}else{
				size_t out_left=BYTES_LEN;
				codetype = conv.conv(s,in_left,out,out_left,codetype);
				int outlen=BYTES_LEN - out_left;
				//thulac::get_raw(oriRaw,out,outlen); 
				thulac::get_raw(oriRaw,out,outlen);
			}
		}
		preprocesser->cleanSpace(oriRaw, raw, poc_cands);
		/*
		if(useT2S){
			preprocesser->cleanAndT2S(oriRaw,raw,poc_cands);
		}else{
			preprocesser->clean(oriRaw,raw,poc_cands);
		}
		*/
		/*
		for(int j = 0; j < poc_cands.size(); j ++){
			std::cout<<poc_cands[j]<<" ";
		}
		std::cout<<std::endl;
		*/

		if(raw.size()){
			tagging_decoder->segment(raw, poc_cands, tagged);
			//tagging_decoder->output_allow_tagging();
			ns_dict->adjustSame(tagged);
			idiom_dict->adjustSame(tagged);
			nz_dict->adjustSame(tagged);
			noun_dict->adjustSame(tagged);
			vm_dict->adjustSame(tagged);
			y_dict->adjustSame(tagged);
			if(user_dict){
				user_dict->adjust(tagged);
			}
			punctuation->adjust(tagged);
			timeword->adjust(tagged);
			verbword->adjust(tagged);
			if(useFilter){
				filter->adjust(tagged);
			}
			if(codetype==0){
				std::cout<<tagged;
			}else{
				ost<<tagged;
				std::string str=ost.str();
				strcpy(s,str.c_str());
				size_t in_left=str.size();
				size_t out_left=BYTES_LEN;
				codetype = conv.invert_conv(s,in_left,out,out_left,codetype);
				int outlen=BYTES_LEN - out_left;
				std::cout<<std::string(out,outlen);
				ost.str("");
			}
			/*
			if(i != sent_vec.size() - 1){
				putchar(32);
			}
			*/
		}
		putchar(10);
		std::cout.flush();
	}

	
	clock_t end = clock();
	double duration = (double)(end - start) / CLOCKS_PER_SEC;
	std::cerr<<duration<<" seconds"<<std::endl;
	

	delete [] s;
	delete [] out;

	delete preprocesser;
	delete ns_dict;
	delete idiom_dict;
	delete nz_dict;
	delete noun_dict;
	if(user_dict != NULL){
		delete user_dict;
	}

	delete timeword;
	delete verbword;
	delete punctuation;
	if(useFilter){
		delete filter;
	}

	delete tagging_decoder;
	if(tagging_model != NULL){
		for(int i = 0; i < tagging_model->l_size; i ++){
			if(tagging_label_info) delete[](tagging_label_info[i]);
		}
	}   
	delete[] tagging_label_info;
     
	if(tagging_pocs_to_tags){
		for(int i = 1; i < 16; i ++){
			delete[] tagging_pocs_to_tags[i];
		}
	}
	delete[] tagging_pocs_to_tags;

	delete tagging_dat;
	if(tagging_model!=NULL) delete tagging_model;

	return 0;
}


