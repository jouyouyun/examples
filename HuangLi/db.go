/**
 * Description: 使用 SQLite 存储黄历信息
 *
 * Author: jouyouyun <jouyouwen717@gmail.com>
 *
**/
package main

import (
	"database/sql"

	_ "github.com/mattn/go-sqlite3"
)

// HuangLi huang li info from baidu
type HuangLi struct {
	ID    int64  `json:"id"` // format: ("%s%02s%02s", year, month, day)
	Avoid string `json:"avoid"`
	Suit  string `json:"suit"`
}

// HuangLiList huang li info list
type HuangLiList []*HuangLi

var (
	_db *sql.DB
)

// Init open db and create table
func Init(filename string) error {
	var err error
	_db, err = sql.Open("sqlite3", filename)
	if err != nil {
		return err
	}
	tableStmt := `
CREATE TABLE IF NOT EXISTS huangli (id INTEGER NOT NULL PRIMARY KEY, avoid TEXT, suit TEXT);
`
	_, err = _db.Exec(tableStmt)
	if err != nil {
		return err
	}
	return nil
}

// Finalize close db
func Finalize() {
	_ = _db.Close()
}

// Create insert to sqlite, if exists, ignore
func (list HuangLiList) Create() error {
	tx, err := _db.Begin()
	if err != nil {
		return err
	}

	for _, info := range list {
		tmp, _ := txQuery(tx, info.ID)
		if tmp != nil {
			// fmt.Println("Has exists:", tmp.ID, tmp.Avoid, tmp.Suit, info.ID, info.Avoid, info.Suit)
			continue
		}
		err = txCreate(tx, info)
		if err != nil {
			tx.Rollback()
			return err
		}
	}

	return tx.Commit()
}

// NewHuangLi query by id
func NewHuangLi(id int64) (*HuangLi, error) {
	stmt, err := _db.Prepare("SELECT id, avoid, suit FROM huangli WHERE id = ?")
	if err != nil {
		return nil, err
	}
	defer stmt.Close()

	var info HuangLi
	err = stmt.QueryRow(id).Scan(&info.ID, &info.Avoid, &info.Suit)
	if err != nil {
		return nil, err
	}
	return &info, nil
}

func txQuery(tx *sql.Tx, id int64) (*HuangLi, error) {
	stmt, err := tx.Prepare("SELECT id, avoid, suit FROM huangli WHERE id = ?")
	if err != nil {
		return nil, err
	}
	defer stmt.Close()

	var info HuangLi
	err = stmt.QueryRow(id).Scan(&info.ID, &info.Avoid, &info.Suit)
	if err != nil {
		return nil, err
	}
	return &info, nil
}

func txCreate(tx *sql.Tx, info *HuangLi) error {
	stmt, err := tx.Prepare("INSERT INTO huangli (id,avoid,suit) VALUES (?,?,?)")
	if err != nil {
		return err
	}
	defer stmt.Close()

	_, err = stmt.Exec(info.ID, info.Avoid, info.Suit)
	return err
}
