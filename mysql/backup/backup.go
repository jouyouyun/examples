package main

import (
	"database/sql"
	"fmt"
	_ "github.com/go-sql-driver/mysql"
	"io/ioutil"
	"os"
	"os/exec"
	"sort"
	"sync"
	"time"
)

type Backup struct {
	conf   *Config
	locker sync.Mutex
}

func NewBackup(conf *Config) (*Backup, error) {
	err := os.MkdirAll(conf.BackupDir, 0755)
	if err != nil {
		return nil, err
	}
	var bak = Backup{conf: conf}
	return &bak, nil
}

func (bak *Backup) StartLoop() {
	// Will backup at 02:00 ~ 03:00
	// fixed time
	t := time.Now()
	if bak.conf.Debug {
		fmt.Println("[Debug] now:", t.String())
	}
	if *test {
		fmt.Println("[Info] SQL source:", bak.conf.GenSQLSource())
		bak.Backup()
		return
	}
	if 2-t.Hour() > 0 {
		time.Sleep(time.Hour * time.Duration(3-t.Hour()))
	} else if 24-t.Hour() < 22 {
		hours := time.Duration(24 - t.Hour() + 1)
		time.Sleep(time.Hour * hours)
	}
	for {
		bak.Backup()
		time.Sleep(time.Hour * 24)
	}
}

func (bak *Backup) Backup() {
	err := bak.doBackup()
	if err != nil {
		fmt.Println("[Error] Failed to exec backup:", err)
	}
	bak.Clean()
}

func (bak *Backup) Clean() {
	names, err := bak.getFileList()
	if err != nil {
		fmt.Println("[Error] Failed to get file list:", err)
		return
	}
	nlen := len(names)
	if nlen <= bak.conf.RetentionNum {
		return
	}
	sort.Strings(names)
	idx := nlen - bak.conf.RetentionNum
	for i := 0; i < idx; i++ {
		err = os.Remove(names[i])
		if err != nil {
			fmt.Println("[Error] Failed to remove file:", err)
		}
	}
}

func (bak *Backup) getFileList() ([]string, error) {
	finfos, err := ioutil.ReadDir(bak.conf.BackupDir)
	if err != nil {
		return nil, err
	}
	var names []string
	for _, finfo := range finfos {
		if finfo.IsDir() {
			continue
		}
		names = append(names, bak.conf.BackupDir+"/"+finfo.Name())
	}
	return names, nil
}

func (bak *Backup) doBackup() error {
	bak.locker.Lock()
	defer bak.locker.Unlock()
	if bak.conf.Debug {
		fmt.Println("[Debug] start to do backup")
	}
	db, err := sql.Open("mysql", bak.conf.GenSQLSource())
	if err != nil {
		return err
	}
	defer db.Close()
	db.Exec("FLUSH TABLES WITH READ LOCK")
	defer db.Exec("UNLOCK TABLES")
	cmd := bak.conf.GenSQLDump(bak.genFilename())
	if bak.conf.Debug {
		fmt.Println("[Debug] Backup will exec:", cmd)
	}
	outs, err := exec.Command("/bin/sh", "-c", cmd).CombinedOutput()
	if err != nil {
		return fmt.Errorf("%s(%s)", err.Error(), string(outs))
	}
	return nil
}

func (bak *Backup) genFilename() string {
	return fmt.Sprintf("%s/backup_%s.db", bak.conf.BackupDir,
		time.Now().Format(time.RFC3339))
}
