package main

func main() {
  switch tag {
  case 0, 1, 2, 3: s1()
  case 4, 5, 6, 7: s2()
  default: s3()
  }

  switch x := f(); {  // missing switch expression means "true"
  case x < 0: return -x
  default: return x
  }

  switch {
  case x < y: f1()
  case x < z: f2()
  case x == 4: f3()
  }
}
