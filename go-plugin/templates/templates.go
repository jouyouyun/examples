package templates

type Templates interface {
	Name() string
	Description() string
	Parameters() ([]byte, error)
	Start(params []byte) error
	Stop() error
}

type Database struct {
	Addr      string
	Port      int32
	User      string
	Password  string
	DBName    string
	TableName string
}

type Server struct {
	Port int32
}
