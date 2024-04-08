#include <fstream>
#include <stdio.h>
#include <string>
#include <iostream>
#include "json/json.h"
#include "test.h"
#include <U8g2lib.h>
#include <map>
#include "subprocess/subprocess.hpp"
#define LCDWidth                        u8g2.getDisplayWidth()
#define ALIGN_CENTER(t)                 ((LCDWidth - (u8g2.getUTF8Width(t))) / 2)
#define ALIGN_RIGHT(t)                  (LCDWidth -  u8g2.getUTF8Width(t))
#define ALIGN_LEFT                      0
static U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/
		U8X8_PIN_NONE);

namespace device{
	void u8g2_init(int bus=0,int addr = 0x3c*2){
		u8g2.initI2cHw(bus);
		u8g2.setI2CAddress(addr);
		u8g2.setBusClock(4000000);
		u8g2.begin();
		u8g2.enableUTF8Print();
	}
	
	const uint8_t* find_font(std::string fontname){
		return fontmap[fontname];
	}
}

Json::Value read_config(std::string path){
	Json::Value root;
	std::ifstream config_doc(path, std::ifstream::binary);
	config_doc >> root;
	printf("Using config %s",root["name"].asString().c_str());
	return root;
}

void select_type(Json::Value content){
	std::string data_type = content["data_source"].asString();
	if(data_type == "text"){
		const uint8_t* font = device::find_font(content["contents"]["font"].asString());
		if(font){
			u8g2.setFont(font);// choose a suitable font
		}
		if(content["x_pos"].asString()=="center"){
			u8g2.drawUTF8(ALIGN_CENTER(content["contents"]["text"].asCString()), content["y_pos"].asInt(), content["contents"]["text"].asCString());
		}else if(content["x_pos"].asString()=="right"){
			u8g2.drawUTF8(ALIGN_RIGHT(content["contents"]["text"].asCString()), content["y_pos"].asInt(), content["contents"]["text"].asCString());	
		}else{
			u8g2.drawUTF8(content["x_pos"].asInt(), content["y_pos"].asInt(), content["contents"]["text"].asCString());	
		}
		return;
	}
	if(data_type == "exec"){
		const uint8_t* font = device::find_font(content["contents"]["font"].asString());
		subprocess::popen cmd(content["contents"]["path"].asString(),{content["contents"]["command"].asString()});
		cmd.wait();
		if(font){
			u8g2.setFont(font);// choose a suitable font
		}
		std::stringstream strout;
		strout << cmd.out().rdbuf();
		if(content["x_pos"].asString()=="center"){
			u8g2.drawUTF8(ALIGN_CENTER(strout.str().c_str()), content["y_pos"].asInt(), strout.str().c_str());
		}else if(content["x_pos"].asString()=="right"){
			u8g2.drawUTF8(ALIGN_RIGHT(strout.str().c_str()), content["y_pos"].asInt(), strout.str().c_str());	
		}else{
			u8g2.drawUTF8(content["x_pos"].asInt(), content["y_pos"].asInt(), strout.str().c_str());	
		}
		cmd.close();
		return;	
	}
	if(data_type == "file"){
		const uint8_t* font = device::find_font(content["contents"]["font"].asString());
		std::ifstream config_doc(content["contents"]["path"].asString(), std::ifstream::binary);
		if(font){
			u8g2.setFont(font);// choose a suitable font
		}
		std::stringstream out;
		out << config_doc.rdbuf();
		if(content["x_pos"].asString()=="center"){
			u8g2.drawUTF8(ALIGN_CENTER(out.str().c_str()), content["y_pos"].asInt(), out.str().c_str());
		}else if(content["x_pos"].asString()=="right"){
			u8g2.drawUTF8(ALIGN_RIGHT(out.str().c_str()), content["y_pos"].asInt(), out.str().c_str());	
		}else{
			u8g2.drawUTF8(content["x_pos"].asInt(), content["y_pos"].asInt(), out.str().c_str());	
		}
		return;	
	}
	if(data_type == "bitmap"){
		//content.asString()
	}
}

void fill_display(Json::Value data){
	for ( int index = 0; index < data.size(); ++index ){
		select_type(data[index]);
	}
}

int main(int argc, char* argv[]) {
	Json::Value config;
	int count = 0;
	int colormode = 1;
	if(argv[1]){
		if(argv[1] == "stop"){
			u8g2.setPowerSave(1);
    	    u8g2.doneI2c();
     	    u8g2.doneUserData();
		}
		std::string strin(argv[1]);
		config = read_config(strin);
	}else{
		printf("config file needed");
		return 0;
	}
	device::u8g2_init(config["display_settings"]["bus"].asInt(),config["display_settings"]["addr"].asInt() * 2);
	int refresh = config["display_settings"]["refresh"].asInt();
	int protect = config["display_settings"]["protect_interval"].asInt();
	int duration = config["display_settings"]["protect_duration"].asInt();
	while(true){
	u8g2.clearBuffer();
	if(count >= protect*1000){
		if(colormode==0){
			colormode=1;
			count = 0;
		}else{
			colormode=0;
			count = (protect-duration)*1000;
		}
	}
	u8g2.setDrawColor(!colormode);
	u8g2.drawBox(0,0,128,64);
	u8g2.setDrawColor(colormode);
	fill_display(config["display_widget"]);
	u8g2.sendBuffer();
	count += refresh;
	u8g2.sleepMs(refresh);
	}
}