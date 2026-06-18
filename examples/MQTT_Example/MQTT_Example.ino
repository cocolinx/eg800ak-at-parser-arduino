//
//    FILE: MQTT_Example.ino
//  AUTHOR: CocoLinx
// PURPOSE: Test MQTT communication by publishing 30 messages and then stopping
//     URL: https://github.com/cocolinx/eg800ak-at-parser

#include <CocoLinx_EG800AK_v1.h>

CocoLinx_EG800AK test;

void setup() {
	int32_t ret;

	Serial.begin(115200);
	while (!Serial) ; //
	Serial.println();
	Serial.println("====== setup() start ======");

  Serial.print("cocolinx begin...");
  ret = test.begin();
  if(!ret)
  {
    Serial.println("error");
		Serial.println("halt forever...");
		while (1);
  } 
  else Serial.println("okay");

  ///////////////////// lte connection
  Serial.print("set cfun...");
  ret = test.set_cfun(1);
  if(ret < 0)
  {
    Serial.print("error: ");
    Serial.println(ret);
  }
  else
  {
    Serial.println("okay");
  }

  delay(1000);

  Serial.print("set cereg...");
  ret = test.set_cereg(1);
  if(ret < 0)
  {
    Serial.print("error: ");
    Serial.println(ret);
  }
  else
  {
    Serial.println("okay");
  }

  delay(1000);

  uint32_t start = millis();
  bool connected = false;
  while(millis() - start < 10000)
  {
    Serial.print("read cereg...");  
    int32_t stat;
    ret = test.get_cereg(&stat);
    if(ret < 0)
    {
      Serial.print("error: ");
      Serial.println(ret);
    }
    else 
    {
      Serial.println("okay");
      Serial.print("get_cereg: ");
      Serial.println(stat);
      if(stat == 5) 
      {
        connected = true;
        break;
      }
    }
    delay(1000);  
  }

  if(!connected)
  {
    Serial.println("lte connection failed");
    Serial.println("halt forever");
    while (1);
  }

  ///////////////////// mqtt config
  Serial.print("set mqtt version cfg...");
  ret = test.cx_mqtt_cfg_version(0, 3); // MQTT protocol v3.1
  if(ret < 0)
  {
    Serial.print("error: ");
    Serial.println(ret);
  }
  else Serial.println("okay");

  Serial.print("set mqtt recvmode cfg...");
  ret = test.cx_mqtt_cfg_recvmode(0, 1, 1); // Disable MQTT RX payload in URC and store messages in buffer with length enabled
  if(ret < 0)
  {
    Serial.print("error: ");
    Serial.println(ret);
  }
  else Serial.println("okay");

  ///////////////////// mqtt open
  Serial.print("mqtt server open...");
  ret = test.cx_mqtt_open(0, "broker.emqx.io", 1883);
  if(ret < 0)
  {
    Serial.print("error: ");
    Serial.println(ret);
  }
  else Serial.println("okay...");

  delay(2000);

  ///////////////////// mqtt connect
  Serial.print("mqtt connect...");
  ret = test.cx_mqtt_conn(0, "cocolinx", NULL, NULL);
  if(ret < 0)
  {
    Serial.print("error: ");
    Serial.println(ret);
  }
  else Serial.println("okay...");

  delay(2000);

  ///////////////////// mqtt subscribe
  Serial.print("mqtt topic \"cocolinx/test\" subscribe...");
  ret = test.cx_mqtt_sub(0, 1, "cocolinx/test", 1);
  if(ret < 0)
  {
    Serial.print("error: ");
    Serial.println(ret);
  }
  else Serial.println("okay...");
  delay(2000);
}

void loop() {
  static uint32_t pubCount = 0;
  int32_t ret;

  ///////////////////// mqtt publish
  Serial.print("mqtt topic \"cocolinx/test\" publish...");
  char mqttTx[] = "hello mqtt cocolinx~~";
  ret = test.cx_mqtt_pub(0, 1, 1, 0, "cocolinx/test", mqttTx, strlen(mqttTx));
  if(ret < 0)
  {
    Serial.print("error: ");
    Serial.println(ret);
  }
  else 
  {
    pubCount++;
    Serial.println("okay...");
  }

  delay(2000);

  ///////////////////// mqtt recv buffer status check
  Serial.print("mqtt recv store status check...");
  int32_t store_status[5];
  ret = test.cx_mqtt_recv_info(NULL, store_status, 5);
  if(ret < 0)
  {
    Serial.print("error: ");
    Serial.println(ret);
  }
  else 
  {
    Serial.println("okay...");
    for(int i=0; i<5; i++)
    {
      Serial.print("store status[");
      Serial.print(i);
      Serial.print("]: ");
      Serial.print(store_status[i]);
      Serial.print(" ");
    } 
    Serial.print("\n");
  }

  delay(1000);

  ///////////////////// mqtt recv from buffer
  Serial.print("mqtt recv msg from buffer...");
  char mqttrecv[32];
  int32_t rxcnt = test.cx_mqtt_recv(0, 0, NULL, 0, mqttrecv, sizeof(mqttrecv));
  if(rxcnt < 0)
  {
    Serial.print("error: ");
    Serial.println(rxcnt);
  }
  else 
  {
    Serial.println("okay...");
    Serial.print("recv> ");
    mqttrecv[rxcnt] = '\0';
    Serial.println(mqttrecv);
  }

  if(pubCount >= 30)
  {
    ///////////////////// mqtt disconnect
    Serial.print("mqtt disconnect...");
    ret = test.cx_mqtt_disconn(0);
    if(ret < 0)
    {
      Serial.print("error: ");
      Serial.println(ret);
    }
    else Serial.println("okay...");

    Serial.println("MQTT test finished");
    while(1);
  }

  delay(10000);
}
