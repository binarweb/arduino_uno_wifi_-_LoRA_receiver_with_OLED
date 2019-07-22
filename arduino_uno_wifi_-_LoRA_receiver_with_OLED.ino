#include "src/LoRa/LoRa.h"
#include "src/U8glib/U8glib.h"

// setari
#define SERIAL_BAUDRATE 115200

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_DEV_0);	// I2C / TWI

const long frequency = 433E6;  // LoRa Frequency
const int csPin = 6;           // LoRa radio chip select
const int resetPin = 7;        // LoRa radio reset
// const int resetPin = -1;       // LoRa radio reset
const int irqPin = 2;          // change for your board; must be a hardware interrupt pin
const long SPIfrequency = 8E6; // SPI Frequency

#define OLED_BUFFER_SIZE 20
char incoming_message1[OLED_BUFFER_SIZE] = {0};
char incoming_message2[OLED_BUFFER_SIZE] = {0};
char incoming_message3[OLED_BUFFER_SIZE] = {0};
bool new_message = false;

void draw(void) {
	// graphic commands to redraw the complete screen should be placed here  
	u8g.setFont(u8g_font_unifont);
	u8g.drawStr(1, 10, incoming_message1);
	u8g.drawStr(1, 25, incoming_message2);
	u8g.drawStr(1, 40, incoming_message3);

	// u8g.setFont(u8g_font_osb21);
	// u8g.drawStr(1, 30, incoming_message1);
	// u8g.drawStr(1, 60, incoming_message2);
}

void setup() {
	Serial.begin(SERIAL_BAUDRATE);

	LoRa.setPins(csPin, resetPin, irqPin);
	// LoRa.setSPIFrequency(SPIfrequency);

	if (!LoRa.begin(frequency)) {
		Serial.println("LoRa init failed. Check your connections.");
		while(1); // if failed, do nothing
	}

	// assign default color value
	if ( u8g.getMode() == U8G_MODE_R3G3B2 ) {
		u8g.setColorIndex(255);     // white
	} else if ( u8g.getMode() == U8G_MODE_GRAY2BIT ) {
		u8g.setColorIndex(3);         // max intensity
	} else if ( u8g.getMode() == U8G_MODE_BW ) {
		u8g.setColorIndex(1);         // pixel on
	} else if ( u8g.getMode() == U8G_MODE_HICOLOR ) {
		u8g.setHiColorByRGB(255,255,255);
	}

	pinMode(8, OUTPUT);

	strcpy(incoming_message1, "Waiting for ");
	strcpy(incoming_message2, "a signal to");
	strcpy(incoming_message3, "receive...");
	new_message = true;

	Serial.println("LoRa init succeeded.");
}

void loop() {
	if (new_message) {
		u8g.firstPage();  
		do {
			draw();
		}
		while( u8g.nextPage() );

		new_message = false;
	}

	onReceive(LoRa.parsePacket());
}

void onReceive(int packetSize) {
	if (packetSize == 0) return;          // if there's no packet, return
	
	int i, j, k, n;
	char char_array[OLED_BUFFER_SIZE];

	// message
	memset(incoming_message1, 0, OLED_BUFFER_SIZE);
	i = 0;
	while (LoRa.available()) {
		incoming_message1[i] = (char)LoRa.read();
		i++;
	}

	// RSSI
	memset(incoming_message2, 0, OLED_BUFFER_SIZE);
	strcpy(incoming_message2, "RSSI: ");
	j = strlen(incoming_message2);

	String rssi = String(LoRa.packetRssi());
	n = rssi.length();
	// char char_array[n + 1];
	memset(char_array, 0, OLED_BUFFER_SIZE);
	strcpy(char_array, rssi.c_str());

	for (k = 0; k < n + 1; k++) {
		incoming_message2[j] = char_array[k];
		j++;
	}

	// SNR
	memset(incoming_message3, 0, OLED_BUFFER_SIZE);
	strcpy(incoming_message3, "SNR: ");
	j = strlen(incoming_message3);

	String snr = String(LoRa.packetSnr());
	n = snr.length();
	// char char_array[n + 1];
	memset(char_array, 0, OLED_BUFFER_SIZE);
	strcpy(char_array, snr.c_str());

	for (k = 0; k < n + 1; k++) {
		incoming_message3[j] = char_array[k];
		j++;
	}

	// serial print
	Serial.println("Message: " + String(incoming_message1));
	Serial.println("RSSI: " + String(LoRa.packetRssi()));
	Serial.println("Snr: " + String(LoRa.packetSnr()));
	Serial.println();

	new_message = true;
}