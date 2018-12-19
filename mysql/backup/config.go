package main

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
)

type Config struct {
	User         string `json:"user"`
	Passwd       string `json:"password"`
	Protocol     string `json:"protocol"`
	Host         string `json:"host"`
	Port         int    `json:"port"`
	BackupDir    string `json:"backup_dir"`
	RetentionNum int    `json:"retention_num"`
	Debug        bool   `json:"debug"`
}

func (conf *Config) GenSQLSource() string {
	/*DSN数据源名称
	  [username[:password]@][protocol[(address)]]/dbname[?param1=value1]
	  user@unix(/path/to/socket)/dbname
	  user:password@tcp(localhost:5555)/dbname?charset=utf8&autocommit=true
	  user:password@tcp([de:ad:be:ef::ca:fe]:80)/dbname?charset=utf8mb4,utf8
	  user:password@/dbname
	  无数据库: user:password@/
	*/
	return fmt.Sprintf("%s:%s@%s(%s:%d)/", conf.User, conf.Passwd,
		conf.Protocol, conf.Host, conf.Port)
}

func (conf *Config) GenSQLDump(filename string) string {
	return fmt.Sprintf("mysqldump -h%s --port %d -u%s -p%s --all-databases > %q",
		conf.Host, conf.Port, conf.User, conf.Passwd, filename)
}

func loadConfig(filename string) (*Config, error) {
	contents, err := ioutil.ReadFile(filename)
	if err != nil {
		return nil, err
	}
	var conf Config
	err = json.Unmarshal(contents, &conf)
	if err != nil {
		return nil, err
	}
	return &conf, nil
}
