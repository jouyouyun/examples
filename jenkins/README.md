# Jenkins API 使用

`Jenkins` 是一款流行的开源持续集成工具，可以用来做一些软件开发的自动化工作，如打包，测试，自动部署等。

`Jenkins` 中有 `view` 和 `job` 的概念， `view` 相当于组， `job` 则是具体的任务。
`view` 下面可以创建 `job` ，但 `job` 可以不在任何 `view` 下。

这里主要介绍 `Jenkins` 提供的 `HTTP API` ，至于如何使用 `Jenkins` 请参看 [Jenkins User Documentation](https://jenkins.io/doc/)。


## API


### 鉴权

`Jenkins` 使用 `Baisc Auth` 的权限验证方式，需要传入 `username` 和 `api token` 。
其中 `api token` 需要在用户的设置界面去创建。

但在 `Job` 的远程触发中，可以设置用于远程触发的 `token` (在 `Job` 的配置页面设置)，这样在触发 `Job` 时就不需要传入 `Basic Auth` 了。
远程触发的 `token` 使用 `urlencode` 的方式放在请求的 `body` 中，其原始数据为： `token=<Token Value>`

下面给出两种方式触发 `Job` 的例子：

-   **Basic Auth**

    `curl -X POST <jenkins url>/view/<view name>/job/<job name>/build --user <username>:<api token>`

-   **Token**

    `curl -X POST <jenkins url>/view/<view name>/job/<job name>/build --data-urlencode token=<Token Value>`


### View

-   创建

    **API:** `<jenkins url>/createView`

    **curl:** `curl -X POST <jenkins url>/createView?name=<view name> --data-binary "@viewCofnig.xml" -H "Content-Type: text/xml" --auth <u>:<p>`

-   删除

    **API:** `<jenkins url>/view/<view name>/doDelete`

    **curl:** `curl -X POST <jenkins url>/view/<view name>/doDelete --auth <u>:<p>`


-   查询状态

    **API:** `<jenkins url>/view/<view name>/api/json`

    **curl:** `curl -X GET <jenkins url>/view/<view name>/api/json --auth <u>:<p>`

-   查询配置

    **API:** `<jenkins url>/view/<view name>/config.xml`

    **curl:** `curl -X GET <jenkins url>/view/<view name>/config.xml --auth <u>:<p>`

-   更新配置

    **API:** `<jenkins url>/view/<view name>/config.xml`

    **curl:** `curl -X POST <jenkins url>/view/<view name>/config.xml --data-binary "@newCofnig.xml" -H "Content-Type: text/xml" --auth <u>:<p>`


### Job

-   创建

    **API:** `<jenkins url>/createItem`

    **curl:** `curl -X POST <jenkins url>/createItem?name=<job name> --data-binary "@jobConfig.xml" -H "Content-Type: text/xml" --auth <u>:<p>`

-   删除

    **API:** `<jenkins url>/job/<job name>/doDelete`

    **curl:** `curl -X POST <jenkins url>/job/<job name>/doDelete --auth <u>:<p>`

-   查询状态

    **API:** `<jenkins url>/view/<view name>/job/<job name>/api/json`

    **curl:** `curl -X GET <jenkins url>/view/<view name>/job/<job name>/api/json --auth <u>:<p>`

-   启用

    **API:** `<jenkins url>/view/<view name>/job/<job name>/enable`

-   禁用

    **API:** `<jenkins url>/view/<view name>/job/<job name>/disable`

-   查询配置

    **API:** `<jenkins url>/view/<view name>/job/<job name>/config.xml`

    **curl:** `curl -X GET <jenkins url>/view/<view name>/job/<job name>/config.xml --auth <u>:<p>`


-   更新配置

    **API:** `<jenkins url>/view/<view name>/job/<job name>/config.xml`

    **curl:** `curl -X POST <jenkins url>/view/<view name>/job/<job name>/config.xml --data-binary "@newCofnig.xml" -H "Content-Type: text/xml" --auth <u>:<p>`


### Job Build

为了描述方便，这里先定义 `baseURL=<jenkins url>/view/<view name>/job/<job name>` ，下面的 `API` 都需要加上 `baseURL` 才是完整的 =URL=。

-   触发构建

    **API:** `/build`

    支持 `token` 触发，支持填入构建参数，构建参数是在 `Job` 配置页面创建的。

    如使用 `token` 并有字符参数 `branch` 和 `target` 的例子：

    ```shell
    curl -X POST <api> --data-urlencode token=<token value> \
    --data-urlencode json='{"parameters": [{"name": "branch", "value": "master"},{"name":"target","value":"mirror"}]}'
    ```

    触发成功后，会在 `header` 的 `Location` 字段指明 `queue url` ，再通过查询 `queue` 就可获取到 `build id` 了。

-   停止构建

    **API:** `/<id>/stop`

-   删除构建

    **API:** `/<id>/doDelete`

-   构建状态

    **API:** `/<id>/api/json`

-   最后一次构建

    **API:** `/lastBuild/api/json`


## 附录


### 使用 `Go` 编写触发构建的程序

这里只给出关键代码：

```go
// QueueInfo jenkins return's info by queue json api
type QueueInfo struct {
        Executable struct {
                Number int    `json:"number"`
                URL    string `json:"url"`
        } `json:"Executable"`
}

var (
        jenkinsURL   = flag.String("url", "https://ci.deepin.io", "Jenkins site url")
        jenkinsView  = flag.String("view", "", "Jenkins job view")
        jenkinsJob   = flag.String("job", "", "Jenkins job name")
        jenkinsToken = flag.String("token", "", "Jenkins job token")
)

// BuildJob trigger a job build
func BuildJob(params map[string]string) (int, error) {
        var api = *jenkinsURL
        if len(*jenkinsView) != 0 {
                api += fmt.Sprintf("/view/%s", *jenkinsView)
        }
        if len(*jenkinsJob) != 0 {
                api += fmt.Sprintf("/job/%s", *jenkinsJob)
        }
        api += "/build"

        // params must encode by url
        var values = make(url.Values)
        for k, v := range params {
                values.Set(k, v)
        }

        req, err := http.NewRequest(http.MethodPost, api,
                bytes.NewBufferString(values.Encode()))
        if err != nil {
                return -1, err
        }
        // must set 'Content-Type' to 'application/x-www-form-urlencoded'
        req.Header.Set("Content-Type", "application/x-www-form-urlencoded; charset=utf-8")

        resp, err := http.DefaultClient.Do(req)
        if err != nil {
                return -1, err
        }
        if resp.Body != nil {
                defer resp.Body.Close()
                data, err := ioutil.ReadAll(resp.Body)
                if err != nil {
                        fmt.Println("Failed to read response content:", err)
                } else {
                        fmt.Println("Response content:", string(data))
                }
        }
        fmt.Println("Status:", resp.Status)
        if resp.StatusCode < 200 || resp.StatusCode >= 300 {
                fmt.Println("Failed to build job")
                return -1, fmt.Errorf("build job failure")
        }

        fmt.Println("Response headers:", resp.Header)
        queueAPI := resp.Header.Get("Location")
        queue, err := GetQueue(queueAPI)
        if err != nil {
                return -1, err
        }

        return queue.Executable.Number, nil
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
```


### 参考资料

没有在 [Jenkins REST API 文档](https://wiki.jenkins.io/display/JENKINS/Remote+access+API) 中找到过多的 `API` 信息，但在 [python-jenkins](https://opendev.org/jjb/python-jenkins) 的 [代码](https://opendev.org/jjb/python-jenkins/src/branch/master/jenkins/%5F%5Finit%5F%5F.py) 中找到，所以想要了解更多请看代码。 
