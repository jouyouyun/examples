// Jenkins build example
//
// Jenkins APIs:
//     触发构建: <jenkins url>/view/<view name>/job/<job name>/build --user username:token \
//                   --data-urlencode json='{"parameter":[{"name": "param name", "value": "param value"}]}'
//     查询构建结果: <jenkins url>/view/<view name>/job/<job name>/<build id>/api/json
//     最后一次结果: <jenkins url>/view/<view name>/job/<job name>/lastBuild/api/json
// Test with commands:
//     http  -v -f <jenkins url>/view/<view name>/job/<job name>/build \
//                 json='{"parameter": [{"name": "name","value":"jouyouyun"}]}' \
//                 --auth <name>:<auth>
//     curl -X POST <jenkins url>/view/<view name>/job/<job name>/build \
//                 --data-urlenacode json='{"parameter":[{"name": "param name", "value": "param value"}]}' \
//                 --user <name>:<token>
//     curl -X POST <jenkins url>/view/<view name>/job/<job name>/build \
//                 -d 'json=<data by url encoded>' --user <name>:<token>

package main

import (
	"bytes"
	"encoding/json"
	"flag"
	"fmt"
	"io/ioutil"
	"net/http"
	"net/url"
	"os"
)

type BuildStatus struct {
	Building        bool   `json:"building"`
	DisplayName     string `json:"displayName"` // '#'+id
	FullDisplayName string `json:"fullDisplayName"`
	Duration        int    `json:"duration"`
	ID              string `json:"id"`
	QueueID         int64  `json:"queueId"`
	Result          string `json:"result"`
	Timestamp       int64  `json:"timestamp"`
	URL             string `json:"url"`
}

// QueueInfo jenkins return's info by queue json api
type QueueInfo struct {
	Executable struct {
		Number int    `json:"number"`
		URL    string `json:"url"`
	} `json:"Executable"`
}

var (
	username      = flag.String("u", "", "Jenkins username")
	token         = flag.String("t", "", "Jenkins user token in user settings --> API Token")
	jenkinsURL    = flag.String("url", "", "Jenkins url")
	jenkinsView   = flag.String("view", "", "Jenkins view name")
	jenkinsJob    = flag.String("job", "", "Jenkins job name in view")
	jenkinsParams = flag.String("params", "", `Jenkins parameter, json format, such as:
        "[{"name": "param1 name", "value": "param1 value"}, {"name": "param2 name", "value": "param2 value"}...]"
`)
	jenkinsAction  = flag.String("action", "", "Jenkins action, avaliable: trigger or get")
	jenkinsBuildID = flag.Int("bid", -1, "Jenkins build id")
)

func main() {
	flag.Parse()
	if len(os.Args) == 2 && (os.Args[1] == "-h" || os.Args[1] == "--help") {
		flag.Usage()
		return
	}

	if len(*jenkinsURL) == 0 || len(*jenkinsView) == 0 ||
		len(*jenkinsJob) == 0 || len(*jenkinsAction) == 0 {
		fmt.Println("Invalid arguments")
		return
	}

	switch *jenkinsAction {
	case "trigger":
		TriggerJobBuild()
	case "get":
		info, err := GetBuildStatus(*jenkinsBuildID)
		if err != nil {
			return
		}
		fmt.Println("Status:", info)
	default:
		fmt.Println("Invalid action")
		return
	}
}

func TriggerJobBuild() {
	uri := fmt.Sprintf("%s/view/%s/job/%s/build", *jenkinsURL, *jenkinsView, *jenkinsJob)
	params := fmt.Sprintf("{\"parameter\": %s}", *jenkinsParams)
	v := url.Values{}
	v.Add("json", params)
	data := v.Encode()
	fmt.Println("[Post] build uri:", uri)
	fmt.Println("[Post] build params:", params)
	fmt.Println("[Post] build data:", data)

	req, err := http.NewRequest(http.MethodPost, uri,
		bytes.NewBufferString(data))
	if err != nil {
		fmt.Println("Failed to new request:", err)
		return
	}
	req.Header.Set("Content-Type", "application/x-www-form-urlencoded; charset=utf-8")

	if len(*username) != 0 {
		req.SetBasicAuth(*username, *token)
	}
	resp, err := http.DefaultClient.Do(req)
	if err != nil {
		fmt.Println("Failed to do post:", err)
		return
	}
	if resp.Body == nil {
		return
	}
	defer resp.Body.Close()

	content, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		fmt.Println("Failed to read response:", err)
		return
	}
	if resp.StatusCode < 200 || resp.StatusCode >= 300 {
		fmt.Println("Failed to trigger job build:", string(content))
		return
	}

	queueInfo, err := GetQueue(resp.Header.Get("Location"))
	if err != nil {
		fmt.Println("Failed to get queue info:", err)
		return
	}
	fmt.Println("Job build url:", queueInfo.Executable.URL)
}

func LastBuildStatus() (*BuildStatus, error) {
	return GetBuildStatus(-1)
}

func GetBuildStatus(id int) (*BuildStatus, error) {
	idStr := fmt.Sprint(id)
	if id == -1 {
		idStr = "lastBuild"
	}
	uri := fmt.Sprintf("%s/view/%s/job/%s/%s/api/json",
		*jenkinsURL, *jenkinsView, *jenkinsJob, idStr)
	resp, err := http.Get(uri)
	if err != nil {
		fmt.Println("Failed to do get:", err)
		return nil, err
	}
	if resp.Body == nil {
		return nil, err
	}
	defer resp.Body.Close()

	content, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		fmt.Println("Failed to read response:", err)
		return nil, err
	}
	var status BuildStatus
	err = json.Unmarshal(content, &status)
	if err != nil {
		fmt.Println("Failed to unmarshal:", err, string(content))
		return nil, err
	}
	return &status, nil
}

func GetQueue(api string) (*QueueInfo, error) {
	resp, err := http.Get(api + "/api/json")
	if err != nil {
		fmt.Println("Failed to get queue:", err)
		return nil, err
	}
	defer resp.Body.Close()

	data, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		fmt.Println("Failed to read response content:", err)
	}

	fmt.Println("Queue response content:", string(data))
	if resp.StatusCode < 200 || resp.StatusCode >= 300 {
		fmt.Println("Failed to get queue info")
		return nil, fmt.Errorf("get queue info failure")
	}

	var info QueueInfo
	err = json.Unmarshal(data, &info)
	if err != nil {
		return nil, err
	}
	return &info, nil
}

// build status example:
//    {
//        "_class": "hudson.model.FreeStyleBuild",
//        "actions": [
//            {
//                "_class": "hudson.model.ParametersAction",
//                "parameters": [
//                    {
//                        "_class": "hudson.model.StringParameterValue",
//                        "name": "name",
//                        "value": "Jenkins"
//                    }
//                ]
//            },
//            {
//                "_class": "hudson.model.CauseAction",
//                "causes": [
//                    {
//                        "_class": "hudson.model.Cause$UserIdCause",
//                        "shortDescription": "Started by user jouyouyun",
//                        "userId": "jouyouyun",
//                        "userName": "jouyouyun"
//                    }
//                ]
//            },
//            {
//            },
//            {
//            }
//        ],
//        "artifacts": [],
//        "building": false,
//        "description": null,
//        "displayName": "#1",
//        "duration": 147,
//        "estimatedDuration": 96,
//        "executor": null,
//        "fullDisplayName": "test-api #1",
//        "id": "1",
//        "keepLog": false,
//        "number": 1,
//        "queueId": 126916,
//        "result": "FAILURE",
//        "timestamp": 1555565091183,
//        "url": "https://ci.deepin.io/view/~test/job/test-api/1/",
//        "builtOn": "builder-repo",
//        "changeSet": {
//            "_class": "hudson.scm.EmptyChangeLogSet",
//            "items": [],
//            "kind": null
//        },
//        "culprits": []
//    }
