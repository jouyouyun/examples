package main

const (
	JobTypeJenkins = iota + 101
	JobTypeMirrorsStatus
)

const (
	JobStatusWait = iota + 201
	JobStatusRunning
	JobStatusSuccess
	JobStatusFailed
)

type Progress struct {
	JobID      int
	Status     int
	Percentage float64
}
type ProgressList []*Progress

type Job struct {
	ID          int
	TaskID      int
	BuildID     int
	Type        int
	Total       int
	Finish      int
	Failed      int
	Running     bool
	Name        string
	Description string
	Content     map[string]interface{}

	Handle func() error
}
type JobList []*Job

func (job *Job) IsFinished() bool {
	return job.Finish >= job.Total
}

func (job *Job) QueryProgress() *Progress {
	var p = Progress{JobID: job.ID}
	if job.IsFinished() {
		p.Percentage = 100
		if job.Failed > 0 {
			p.Status = JobStatusFailed
		} else {
			p.Status = JobStatusSuccess
		}
		return &p
	}

	if !job.Running {
		p.Status = JobStatusWait
		p.Percentage = 0
		return &p
	}

	p.Status = JobStatusRunning
	p.Percentage = float64(job.Finish) / float64(job.Total)
	if p.Percentage < 1 {
		p.Percentage = 1
	}
	return &p
}
