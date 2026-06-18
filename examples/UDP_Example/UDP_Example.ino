//
//    FILE: MQTT_Example.ino
//  AUTHOR: CocoLinx
// PURPOSE: Test UDP socket communication by send 30 messages and then stopping
//     URL: https://github.com/cocolinx/eg800ak-at-parser-arduino

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

  bool is_pdp_activated = false;

  ///////////////////// check pdp state
  Serial.print("get pdp state...");
  int32_t context_state, context_type;
  ret = test.cx_get_pdp_state(1, &context_state, &context_type);
  if(ret < 0)
  {
    Serial.print("error: ");
    Serial.println(ret);
    return false;
  }
  else if(ret == CocoLinx_EG800AK::ACK_NO_PDP)
  {
    Serial.println("okay");
    Serial.println("no pdp activated");
  }
  else
  {
    Serial.println("okay");
    Serial.print("context state: ");
    Serial.println(context_state);
    Serial.print("context type: ");
    Serial.println(context_type);
    is_pdp_activated = true;
  } 

  if(!is_pdp_activated)
  {
    Serial.print("pdp activate...");
    ret = test.cx_pdp_activate(1);
    if(ret < 0)
    {
      Serial.print("error: ");
      Serial.println(ret);
      return false;
    }
    else Serial.println("okay");
  }

  delay(1000);

  ///////////////////// open udp socket
  Serial.print("open udp socket...");
  ret = test.cx_socket(1, 0, "UDP", "43.200.166.133", 7777);
  if(ret < 0)
  {
    Serial.print("error: ");
    Serial.println(ret);
    return false;
  }
  else Serial.println("okay");
  
  delay(1000);
}

void loop() {
  static uint32_t txcnt = 0;
  int32_t ret;

  Serial.print("udp send...");
  char udpTx[] = "hello udp cocolinx~~";
  ret = test.cx_send(0, udpTx, strlen(udpTx));
  if(ret < 0)
  {
    Serial.print("error: ");
    Serial.println(ret);
    return false;
  }
  else 
  {
    txcnt++;
    Serial.println("okay");
  }

  delay(1500);

  Serial.print("upd recv data...");
  char udpRx[32];
  int32_t rxcnt = test.cx_recv(0, udpRx, sizeof(udpRx) - 1);
  if(rxcnt < 0)
  {
    Serial.print("error: ");
    Serial.println(rxcnt);
    return false;
  }
  else
  {
    Serial.println("okay");
    Serial.print("recv> ");
    udpRx[rxcnt] = '\0';
    Serial.println(udpRx);
  }

  if(txcnt >= 30)
  {
    Serial.print("udp close...");
    ret = test.cx_close(1);
    if(ret < 0)
    {
      Serial.print("error: ");
      Serial.println(ret);
      return false;
    }
    else Serial.println("okay...");
  }
}
