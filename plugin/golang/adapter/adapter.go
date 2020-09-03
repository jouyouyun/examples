package adapter

type Adapter interface {
	Apply() error
	Check() error
}
