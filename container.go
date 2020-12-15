// 容器实现的核心有两点，分别是：
// Namespace  namespace control what you can see 控制容器能看到的东西，比如文件和其它进程
// CGroup control what you can use 控制容器能用的东西，比如网络，CPU等
// 
// 注意：只有 linux 才支持这两点，linux 永远的神

package main

import (
  "fmt"
  "os"
  "os/exec"
  "syscall"
  "path/filepath"
  "io/ioutil"
  "strconv"
)

func main() {
  switch os.Args[1] {
    case "run":
      run()
    case "child":
      child()
    default:
      panic("bad command")
  }
}

func run() {
  fmt.Printf("Running run %v\n", os.Args[2:])
  cmd := exec.Command("/proc/self/exe", append([]string{"child"}, os.Args[3:]...)...)
  cmd.Stdin = os.Stdin
  cmd.Stdout = os.Stdout
  cmd.Stderr = os.Stderr
  // 通过 Cloneflags 来控制能看到的资源
  cmd.SysProcAttr = &syscall.SysProcAttr{
    Cloneflags:   syscall.CLONE_NEWUTS | syscall.CLONE_NEWPID | syscall.CLONE_NEWNS,
    Unshareflags: syscall.CLONE_NEWNS,
   }
  err := cmd.Run()
  if err != nil {
    panic(err)
  }
}

func child() {
  //限制20个进程
  pidControl(20)
  //限制使用的cpu为0.5核
  cpuControl(0.5)
  //other code ...
}

func pidControl(maxPids int) {
  pidCg := "/sys/fs/cgroup/pids"
  groupPath := filepath.Join(pidCg, "/gocg")

  err := os.Mkdir(groupPath, 0775)
  if err != nil && !os.IsExist(err) {
    panic(err)
  }

  must(ioutil.WriteFile(filepath.Join(groupPath, "pids.max"), []byte(strconv.Itoa(maxPids)), 0700))
  // 通过 cgroups 来控制 pid 核心数，将数据通过 VFS 写回
  must(ioutil.WriteFile(filepath.Join(groupPath, "cgroup.procs"), []byte(strconv.Itoa(os.Getpid())), 0700))
}

func cpuControl(core float64) {
  pidCg := "/sys/fs/cgroup/cpu"
  groupPath := filepath.Join(pidCg, "/gocg")
  //创建gocg组
  err := os.Mkdir(groupPath, 0775)
  if err != nil && !os.IsExist(err) {
    panic(err)
  }
  // 10ms
  cfs := float64(10000)
  // cpu配额
  must(ioutil.WriteFile(filepath.Join(groupPath, "cpu.cfs_quota_us"), []byte(strconv.Itoa(int(cfs*core))), 0700))
  // 时间周期
  must(ioutil.WriteFile(filepath.Join(groupPath, "cpu.cfs_period_us"), []byte(strconv.Itoa(int(cfs))), 0700))
  // 将当前进程加入到gocg组
  must(ioutil.WriteFile(filepath.Join(groupPath, "cgroup.procs"), []byte(strconv.Itoa(os.Getpid())), 0700))
}

func must(err error) {
  if err != nil {
    panic(err)
  }
}
