#include "SD.h"
#include "SPI.h"
#include "TFT_eSPI.h"
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <time.h>

#include "font/Open_Sans_Light_90.h"

#include "config.h" 

/*
^ This file should contain the following defines:
#define CONFIG_WIFI_SSID "WIFI SSID"
#define CONFIG_WIFI_PASS "WIFI PASSWORD"
#define CONFIG_NTP_SERVER "NTP SERVER URL"  
#define CONFIG_NTP_TIMEZONE "CET-1CEST,M3.5.0/2,M10.5.0/2"
#define CONFIG_NTP_TIMEZONE_OFFSET 3600 // TIMEZONE OFFSET IN SECONDS
*/

#define CONFIG_DISPLAY_UPDATE_INTERVAL 30000
#define LANDSCAPE_ORIENTATION 1

TFT_eSPI tft = TFT_eSPI();
WiFiUDP wifi_udp;

String time_string, date_string;
String week_days[] = { "Niedziela", "Poniedz.", "Wtorek", "Sroda", "Czwartek", "Piatek", "Sobota" };
int wifi_status[8] = { 
	TFT_WHITE,       // WL_IDLE_STATUS      = 0,
	TFT_DARKGREY,    // WL_NO_SSID_AVAIL    = 1,
	TFT_BLUE,        // WL_SCAN_COMPLETED   = 2,
	TFT_GREEN,       // WL_CONNECTED        = 3,
	TFT_YELLOW,      // WL_CONNECT_FAILED   = 4,
	TFT_ORANGE,      // WL_CONNECTION_LOST  = 5,
	TFT_PURPLE,      // WL_WRONG_PASSWORD   = 6,
	TFT_RED          // WL_DISCONNECTED     = 7
};
    
unsigned long last_update;

void setup() {
	Serial.begin(9600);
	WiFi.begin(CONFIG_WIFI_SSID, CONFIG_WIFI_PASS); 
	tft.init();

	tft.setRotation(LANDSCAPE_ORIENTATION);
	tft.setTextSize(2);
	tft.setTextDatum(MC_DATUM);
	tft.setTextColor(TFT_BLACK);
	tft.fillScreen(TFT_WHITE);

	// init wifi
	{
		tft.setCursor(16, 16);
		tft.printf("Connecting to WiFi");
		tft.setCursor(16, 32);

		while (WiFi.status() != WL_CONNECTED) {
			delay(200);
			tft.printf(".");
		}
	}

	// init ntp
	{
		tft.setCursor(16, 64);
		tft.printf("Configuring NTP");
		tft.setCursor(16, 80);

		// configure ntp server and timezone
		configTime(0, 0, CONFIG_NTP_SERVER);
		setenv("TZ", CONFIG_NTP_TIMEZONE, 1); 
		tzset();

		bool working = false;
		while (!working) {
			// there are certainly better ways to check if time is set
			time_t current_time;
			time(&current_time);

			tm data = *gmtime(&current_time); 
			working = data.tm_year != 70;

			delay(200);
			tft.printf(".");
		}
	}

	// force first update
	last_update = millis() - CONFIG_DISPLAY_UPDATE_INTERVAL;

	tft.fillScreen(TFT_BLACK);
	tft.setTextColor(TFT_WHITE);
	tft.setFreeFont(&Open_Sans_Light_90);

}

void draw_time() {
	tft.setTextSize(2);
	tft.drawString(time_string, tft.width() / 2, (tft.height() / 2) - 70);

	tft.setTextSize(1);
	tft.drawString(date_string, tft.width() / 2, tft.height() - 75);

	tft.fillRect(4, 320 - 4, 4, 4, wifi_status[WiFi.status()]);
}


void update() {
	tft.setTextColor(TFT_BLACK);
	draw_time();	

	// update time
	{
		time_t current_time;
		time(&current_time);
		current_time += CONFIG_NTP_TIMEZONE_OFFSET;

		tm data = *gmtime(&current_time); 

		String temp_hour = String(data.tm_hour);
		if (temp_hour.length() == 1) temp_hour = "0" + temp_hour;

		String temp_min = String(data.tm_min);
		if (temp_min.length() == 1) temp_min = "0" + temp_min;

		time_string = temp_hour + ":" + temp_min;
		date_string = week_days[data.tm_wday];
	}

	tft.setTextColor(TFT_WHITE);
	draw_time();

	last_update = millis();
}

void loop() {
	unsigned long time_difference = millis() - last_update;
	if (time_difference > CONFIG_DISPLAY_UPDATE_INTERVAL) update();

	delay(CONFIG_DISPLAY_UPDATE_INTERVAL);
}

