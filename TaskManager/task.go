package main

import (
	"fmt"
	"time"
)

type Task struct {
	ID          int
	Name        string // upstream
	Description string
	Operator    string
	Email       string

	Jobs JobList
}

type JenkinsBuild struct {
	Name        string
	Description string
	URI         string
	Token       string
}
type JenkinsBuildList []*JenkinsBuild

func (task *Task) IsFinished() bool {
	for _, job := range task.Jobs {
		if !job.IsFinished() {
			return false
		}
	}
	return true
}

func (task *Task) QueryProgress() ProgressList {
	var plist ProgressList
	for _, job := range task.Jobs {
		plist = append(plist, job.QueryProgress())
	}
	return plist
}

func (task *Task) Start() error {
	err := task.createJobs()
	if err != nil {
		return err
	}
	task.start(3)
	return nil
}

func (task *Task) Retry() {
	// reset failed job
	for _, job := range task.Jobs {
		if job.IsFinished() && job.Failed < 1 {
			// success
			continue
		}
		job.Failed = 0
		job.Finish = 0
		err := task.handleJob(job, 3)
		if err != nil {
			fmt.Println("Failed to retry run job:", err)
			// notifify operator
			break
		}
	}
}

func (task *Task) createJobs() error {
	jenkinsList := task.getJenkinsBuildList()
	if len(jenkinsList) == 0 {
		return fmt.Errorf("invalid task: no jenkins build found")
	}

	// create jobs
	for _, jenkins := range jenkinsList {
		// new and save job
		var job = Job{
			TaskID:      task.ID,
			Type:        JobTypeJenkins,
			Total:       1,
			Name:        jenkins.Name,
			Description: jenkins.Description,
		}
		job.Content = make(map[string]interface{})
		job.Content["uri"] = jenkins.URI
		job.Content["token"] = jenkins.Token
		job.Handle = func() error {
			job.Running = true
			// set build id
			job.BuildID = 1
			// post with uri and token
			fmt.Println("Content:", job.Content)
			// test by sleep
			time.Sleep(time.Second * 90)
			job.Finish++
			return nil
		}
		task.Jobs = append(task.Jobs, &job)

		// mirrors status job
		var mirrors = task.getMirrorList()
		var job2 = Job{
			TaskID:      task.ID,
			Type:        JobTypeMirrorsStatus,
			Total:       len(mirrors),
			Name:        task.Name,
			Description: task.Description,
		}
		job2.Content = make(map[string]interface{})
		job2.Content["mirrors"] = mirrors
		job2.Handle = func() error {
			job2.Running = true
			// set build id
			job.BuildID = 1
			fmt.Println("Content:", job2.Content)
			mirrors := job2.Content["mirrors"].([]string)
			for _, mirror := range mirrors {
				fmt.Println("Start check mirror:", mirror)
				// do check
				time.Sleep(time.Second * 90)
				job2.Finish++
			}
			return nil
		}
		task.Jobs = append(task.Jobs, &job)
	}
	return nil
}

func (task *Task) start(retry int) {
	// blocked
	for _, job := range task.Jobs {
		err := task.handleJob(job, retry)
		if err != nil {
			fmt.Println("Failed to run job:", err)
			// notifify operator
			break
		}
	}
}

func (task *Task) handleJob(job *Job, retry int) error {
	if job.IsFinished() && job.Failed < 1 {
		// success
		return nil
	}

	if retry == 0 {
		return job.Handle()
	}

	var err error
	for i := 0; i < retry; i++ {
		err = job.Handle()
		if err == nil {
			break
		}
		// retry after duration(10min)
		time.Sleep(time.Minute * 10)
	}
	return err
}

func (task *Task) getJenkinsBuildList() JenkinsBuildList {
	// from config found jenkins list by name

	// test data
	return JenkinsBuildList{
		&JenkinsBuild{
			Name:        "community_repo_sync",
			Description: "Sync comminuty repository",
			URI:         "https://ci.deepin.io/view/test~/job/comminuty_repo_sync",
			Token:       "deepin",
		},
	}
}

func (task *Task) getMirrorList() []string {
	// get mirror list by task name

	// test data
	return []string{
		"https://packages.deepin.com",
		"https://mirrors.ustc.edu.cn/deepin",
		"https://mirrors.163.com/deepin",
	}
}
