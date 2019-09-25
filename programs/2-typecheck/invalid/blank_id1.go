package main // Only the last statement is supposed to be incorrect. Blank increment


func _(_ int, _ float64) int {
  return 0
}


func _() float64 {
  return 0.0
}
func main() {
  var _ int
  var _ float64
  type _ int
  type _ struct {
    _ int
    _ float64
  }
  _, a := 0, 0
  _++;
}

