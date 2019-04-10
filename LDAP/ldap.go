package main

import (
	"crypto/tls"
	"fmt"
	"time"

	"flag"
	ldapBase "gopkg.in/ldap.v3"
)

type Client struct {
	conn     *ldapBase.Conn
	dn       string
	password string
	search   string
}

var (
	lhost   = flag.String("lh", "", "LDAP Host")
	lport   = flag.Int("lp", 389, "LDAP Port")
	ldn     = flag.String("ldn", "", "LDAP DN")
	lpasswd = flag.String("lpw", "", "LDAP Password")
	lsearch = flag.String("ls", "", "LDAP Search")

	username = flag.String("u", "", "user")
	password = flag.String("p", "", "password")

	luseTls = flag.Bool("lup", true, "enable tls")
)

func main() {
	flag.Parse()
	cli, err := NewClient(*lhost, *lport, *ldn, *lpasswd, *lsearch, *luseTls)
	if err != nil {
		fmt.Println("Failed to dial server:", err)
		return
	}
	defer cli.conn.Close()

	err = cli.CheckUserPassword(*username, *password)
	if err != nil {
		fmt.Println("Failed to check user:", err)
		return
	}
}

func NewClient(host string, port int, dn, password, search string,
	useTls bool) (client *Client, err error) {
	var conn *ldapBase.Conn

	ldapBase.DefaultTimeout = 20 * time.Second
	if useTls {
		conn, err = ldapBase.DialTLS("tcp", host+":"+fmt.Sprint(port),
			&tls.Config{InsecureSkipVerify: true})
	} else {
		conn, err = ldapBase.Dial("tcp", host+":"+fmt.Sprint(port))
	}
	if err != nil {
		return
	}
	client = &Client{
		conn:     conn,
		dn:       dn,
		password: password,
		search:   search,
	}
	return
}

func (c *Client) prepare() (err error) {
	err = c.conn.Bind(c.dn, c.password)
	if err != nil {
		err = fmt.Errorf("LDAP search is not set properly")
	}
	return
}

func (c *Client) CheckUserPassword(username, password string) (err error) {
	err = c.prepare()
	if err != nil {
		return
	}
	req := ldapBase.NewSearchRequest(
		c.search, ldapBase.ScopeWholeSubtree, ldapBase.NeverDerefAliases,
		0, 0, false,
		fmt.Sprintf("(uid=%s)", ldapBase.EscapeFilter(username)),
		[]string{"dn"}, nil)
	resp, err := c.conn.Search(req)
	if err != nil {
		return
	}
	if len(resp.Entries) != 1 {
		err = fmt.Errorf("ldap failed to match")
		return
	}
	userDn := resp.Entries[0].DN
	err = c.conn.Bind(userDn, password)
	if err != nil {
		err = fmt.Errorf("invalid username or password")
		return
	}
	return
}
