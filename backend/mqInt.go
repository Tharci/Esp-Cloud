package main

import (
	"fmt"
	"net/http"
	"io/ioutil"
	"log"
	"time"
	"strconv"
	"encoding/json"

	mqtt "github.com/eclipse/paho.mqtt.golang"

	//"database/sql"
	_ "github.com/denisenkom/go-mssqldb"

	"container/list"
)

type device struct {
    name string
	id  string
	data string
	confirmed int64
}

func findDevice(deviceId string) *device {
	for d := devices.Front(); d != nil; d = d.Next() {
		if d.Value.(*device).id == deviceId {
			return d.Value.(*device)
		}
	}
	return nil
}

var mqClient mqtt.Client
var devices *list.List

func main() {
	devices = list.New()

	// MQTT
	opts := mqtt.NewClientOptions().AddBroker("mqtt.tharci.fail:8883")
	opts.SetClientID("MQTTInterface")
	opts.SetDefaultPublishHandler(f)

	//create and start a client using the above ClientOptions
	fmt.Println("Connecting to MQTT server...")
	mqClient = mqtt.NewClient(opts)
	if token := mqClient.Connect(); token.Wait() && token.Error() != nil {
		fmt.Println("Failed to connect to MQTT server.")
		panic(token.Error())
	}
	fmt.Println("Successfully connected to MQTT server.")
	
	mqClient.Subscribe("confirm", 0, confirmDevice)

	go deviceGarbageCollector();


	// SQL
	/*fmt.Println("Connecting to SQL server...")
	_, errdb := sql.Open("mssql", 
		"server=https://uurl.ga/sqlmanager/;user id=tharci;password=SA_PASSWORD=eE4Wl3JlCpiFujVE;"
		)

	if errdb != nil {
		fmt.Println("Failed to connect to SQL server: ", errdb.Error())
	} else {
		fmt.Println("Successfully connected to SQL server.")
	}*/


	// HTTP
	http.HandleFunc("/on", lightOn)
	http.HandleFunc("/off", lightOff)
	http.HandleFunc("/light", lightPage)
	http.HandleFunc("/blink", blink)
	
	http.HandleFunc("/iot", iotRequest)
	http.HandleFunc("/iot/getData", iotGetData)
	http.HandleFunc("/iot/publish", publishCallback)

	fmt.Println("Starting HTTP server on port 9080...")
	http.ListenAndServe(":9080", nil)
	fmt.Println("Failed to start HTTP server.")
}

func confirmDevice(client mqtt.Client, msg mqtt.Message) {
	deviceId := string(msg.Payload())
	fmt.Println("\nGot confirm request: " + deviceId)

	/*for d := devices.Front(); d != nil; d = d.Next() {
		if d.Value.(*device).id == deviceId {
			d.Value.(*device).confirmed = time.Now().Unix()
			fmt.Println(deviceId + " confirmed time updated: " + strconv.FormatInt(d.Value.(*device).confirmed, 10))
			break
		}
	}*/

	d := findDevice(deviceId)

	if !(d == nil) {
		d.confirmed = time.Now().Unix()
		fmt.Println(deviceId + " confirmed time updated: " + strconv.FormatInt(d.confirmed, 10))
	}
}

func deviceGarbageCollector()  {
	for true {
		fmt.Println("\nCollecting garbage...")

		var dev *device
		for d := devices.Front(); d != nil; d = d.Next() {
			dev = d.Value.(*device)
			if time.Now().Unix() - dev.confirmed > 120 {
				devices.Remove(d)
				fmt.Println("Device " + dev.id + " has been deleted.")
			}
		}
	
		fmt.Println("Done.")
		time.Sleep(60 * time.Second)
	}
}

func setupResponse(w *http.ResponseWriter, req *http.Request) {
	(*w).Header().Set("Access-Control-Allow-Origin", "*")
    (*w).Header().Set("Access-Control-Allow-Methods", "POST, GET, OPTIONS, PUT, DELETE")
    (*w).Header().Set("Access-Control-Allow-Headers", "Accept, Content-Type, Content-Length, Accept-Encoding, X-CSRF-Token, Authorization")
}

func iotGetData(w http.ResponseWriter, r *http.Request) {
	setupResponse(&w, r)
	//if r.Method == "POST" {
		msg := ""
		for d := devices.Front(); d != nil; d = d.Next() {
			msg += d.Value.(*device).data + "\n"
		}

		fmt.Print("\nData sent: \n" + msg)

		fmt.Fprintf(w, msg)
	//}
}

func publishCallback(w http.ResponseWriter, r *http.Request) {
	setupResponse(&w, r)
	payload, err := ioutil.ReadAll(r.Body)
    if err != nil {
		log.Fatal(err)
		return
	}
	reqBody := string(payload)

	fmt.Println("\nGot Publish Callback request: \n" + reqBody)

	var jsonMap map[string]interface{}
	err = json.Unmarshal([]byte(reqBody), &jsonMap)

	mqClient.Publish(jsonMap["deviceId"].(string) + "/" + jsonMap["topic"].(string), 0, false, jsonMap["msg"].(string))

	fmt.Fprintf(w, "1")
}

func iotRequest(w http.ResponseWriter, r *http.Request) {
	if r.Method == "POST" {
		payload, err := ioutil.ReadAll(r.Body)
        if err != nil {
			log.Fatal(err)
			return
		}
		reqBody := string(payload)

		fmt.Println("\nGot Iot Login request: \n" + reqBody)

		var jsonMap map[string]interface{}
		err = json.Unmarshal([]byte(reqBody), &jsonMap)

		dId := jsonMap["deviceId"].(string)
		dName := jsonMap["deviceName"].(string)
		
		//fmt.Println("%+v\n", jsonMap)

		if jsonMap["msgId"] == "0" {
			d := findDevice(jsonMap["deviceId"].(string))
			delete(jsonMap, "msgId")
			dataByteArray, err := json.Marshal(jsonMap)
			if d == nil {
				// delete(jsonMap, "deviceId")
				if err != nil {
					fmt.Println(err)
				} else {
					data := string(dataByteArray)
					devices.PushBack(&device{name: dName, id: dId, data: data, confirmed: time.Now().Unix()})
					//fmt.Println("\ndata: \n" + data)
					fmt.Println("-> New device registered.")
				}
			} else {
				if err != nil {
					fmt.Println(err)
				} else {
					d.name = dName
					d.data = string(dataByteArray)
					fmt.Println("-> Device already registered.")
				}
			}

			fmt.Fprintf(w, "1")
		} else {
			fmt.Fprintf(w, "0")
		}
	} else {
		fmt.Fprintf(w, "IoT API")
	}
}

var f mqtt.MessageHandler = func(mqClient mqtt.Client, msg mqtt.Message) {
	fmt.Println("Callback called.")
	fmt.Println("TOPIC: %s\n", msg.Topic())
	fmt.Println("MSG: %s\n", msg.Payload())
}

func blink(w http.ResponseWriter, r *http.Request) {
	mqClient.Publish("blink", 0, false, "1")

	fmt.Fprintf(w, "Blinked.")
}

func lightOn(w http.ResponseWriter, r *http.Request) {
	mqClient.Publish("/light", 0, false, "1")

	fmt.Fprintf(w, "Light turned on.")
}

func lightOff(w http.ResponseWriter, r *http.Request) {
	mqClient.Publish("/light", 0, false, "0")

	fmt.Fprintf(w, "Light turned off.")
}

func lightPage(w http.ResponseWriter, r *http.Request) {
	mqClient.Publish("/light", 0, false, "0")

	l26 := r.URL.Query().Get("l26")
	l27 := r.URL.Query().Get("l27")

	if (l26 == "on") {
		fmt.Println("L26 turned on.")
		mqClient.Publish("/light", 0, false, "l26 on")
	} else if (l26 == "off") {
		fmt.Println("L26 turned off.")
		mqClient.Publish("/light", 0, false, "l26 off")
	}

	if (l27 == "on") {
		fmt.Println("L27 turned on.")
		mqClient.Publish("/light", 0, false, "l27 on")
	} else if (l27 == "off") {
		fmt.Println("L27 turned off.")
		mqClient.Publish("/light", 0, false, "l27 off")
	}

	fmt.Fprintf(w, `
		<!DOCTYPE html>
		<html>
		<body>
			<h3>LED 26</h3>
			<p><a href="/light?l26=on">ON</a> &#8195; <a href="/light?l26=off"> OFF</a></p>
			<h3>LED 27</h3>
			<p><a href="/light?l27=on">ON</a> &#8195; <a href="/light?l27=off"> OFF</a></p>
		</body>
		</html>
		`)
}
