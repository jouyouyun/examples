package main

import (
	"flag"
	"fmt"
	"github.com/jinzhu/gorm"
	_ "github.com/jinzhu/gorm/dialects/mysql"
	"time"
)

type User struct {
	ID        int        `gorm:"TYPE:int(11);NOT NULL;PRIMARY_KEY;INDEX"`
	Name      string     `gorm:"TYPE: VARCHAR(255); DEFAULT:'';INDEX"`
	Companies []Company  `gorm:"ForeignKey:UserId;AssociationForeignKey:ID"`
	CreatedAt time.Time  `gorm:"TYPE:DATETIME"`
	UpdatedAt time.Time  `gorm:"TYPE:DATETIME"`
	DeletedAt *time.Time `gorm:"TYPE:DATETIME;DEFAULT:NULL"`
}

type Company struct {
	gorm.Model
	Industry int    `gorm:"TYPE:INT(11);DEFAULT:0"`
	Name     string `gorm:"TYPE:VARCHAR(255);DEFAULT:'';INDEX"`
	Job      string `gorm:"TYPE:VARCHAR(255);DEFAULT:''"`
	UserId   int    `gorm:"TYPE:int(11);NOT NULL;INDEX"`
}

var (
	host   = flag.String("H", "10.0.10.70", "the mysql host")
	port   = flag.Int("P", 3049, "the mysql port")
	user   = flag.String("u", "root", "the mysql user")
	passwd = flag.String("p", "", "the mysql password")
	dbName = flag.String("n", "", "the mysql database name")

	db *gorm.DB
)

func main() {
	flag.Parse()
	if len(*host) == 0 || *port == 0 || len(*user) == 0 ||
		len(*passwd) == 0 || len(*dbName) == 0 {
		fmt.Println("Invalid arguments")
		flag.Usage()
		return
	}
	err := connectDB()
	if err != nil {
		return
	}
	defer db.Close()

	db.Debug().AutoMigrate(&User{})
	db.Debug().AutoMigrate(&Company{})
	db.Debug().Create(&User{ID: 1, Name: "admin"})
	db.Debug().Create(&Company{UserId: 1, Industry: 1, Name: "Jouyouyun"})

	var u User
	// 关联查询
	err = db.Debug().Preload("Companies").First(&u).Error
	if err != nil {
		fmt.Println("Find user failed:", err)
		return
	}
	fmt.Println("User info: ", u, len(u.Companies))

	// 关联查询所有记录
	var list []User
	err = db.Debug().Preload("Companies").Find(&list).Error
	if err != nil {
		fmt.Println("Query all failed:", err)
		return
	}
	for _, v := range list {
		fmt.Println("List one:", v)
	}
}

func connectDB() error {
	source := fmt.Sprintf("%s:%s@tcp(%s:%d)/%s?charset=utf8&parseTime=True&loc=Local",
		*user, *passwd,
		*host, *port, *dbName)
	fmt.Println("Sql source:", source)
	var err error
	db, err = gorm.Open("mysql", source)
	if err != nil {
		fmt.Println("Failed to open mysql:", err)
		return err
	}
	return nil
}
