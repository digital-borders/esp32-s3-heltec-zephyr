## BLE

- accensione vaso
- check if ho un account salvato
- non ho un account associato
- ble sempre acceso
- metto il nome + seriale
- qualcuno si connette a me (utilizzando la nostra app)
- app manda dettagli rete wifi (se disponibile)
- app manda dettagli user
- il vaso

Caratteristiche BLE:

- BatteryService
- DeviceInformation
- CurrentTime
- SetEmail (write-only)
- SetWifiSSID (read + write)
- SetWifiPassword (write)
- Status (read-only) DISCONNECTED, SEARCHING, LORA, WIFI, LORA+WIFI

- Humidity
- Temperature
- SoilMoisture
- Light
- WaterLevel
- WaterPump
- AirQuality

## GATT Format type

#define 	BLE_GATT_CPF_FORMAT_RFU   0x00
 
#define 	BLE_GATT_CPF_FORMAT_BOOLEAN   0x01
 
#define 	BLE_GATT_CPF_FORMAT_2BIT   0x02
 
#define 	BLE_GATT_CPF_FORMAT_NIBBLE   0x03
 
#define 	BLE_GATT_CPF_FORMAT_UINT8   0x04
 
#define 	BLE_GATT_CPF_FORMAT_UINT12   0x05
 
#define 	BLE_GATT_CPF_FORMAT_UINT16   0x06
 
#define 	BLE_GATT_CPF_FORMAT_UINT24   0x07
 
#define 	BLE_GATT_CPF_FORMAT_UINT32   0x08
 
#define 	BLE_GATT_CPF_FORMAT_UINT48   0x09
 
#define 	BLE_GATT_CPF_FORMAT_UINT64   0x0A
 
#define 	BLE_GATT_CPF_FORMAT_UINT128   0x0B
 
#define 	BLE_GATT_CPF_FORMAT_SINT8   0x0C
 
#define 	BLE_GATT_CPF_FORMAT_SINT12   0x0D
 
#define 	BLE_GATT_CPF_FORMAT_SINT16   0x0E
 
#define 	BLE_GATT_CPF_FORMAT_SINT24   0x0F
 
#define 	BLE_GATT_CPF_FORMAT_SINT32   0x10
 
#define 	BLE_GATT_CPF_FORMAT_SINT48   0x11
 
#define 	BLE_GATT_CPF_FORMAT_SINT64   0x12
 
#define 	BLE_GATT_CPF_FORMAT_SINT128   0x13
 
#define 	BLE_GATT_CPF_FORMAT_FLOAT32   0x14
 
#define 	BLE_GATT_CPF_FORMAT_FLOAT64   0x15
 
#define 	BLE_GATT_CPF_FORMAT_SFLOAT   0x16
 
#define 	BLE_GATT_CPF_FORMAT_FLOAT   0x17
 
#define 	BLE_GATT_CPF_FORMAT_DUINT16   0x18
 
#define 	BLE_GATT_CPF_FORMAT_UTF8S   0x19
 
#define 	BLE_GATT_CPF_FORMAT_UTF16S   0x1A
 
#define 	BLE_GATT_CPF_FORMAT_STRUCT   0x1B