//~true
//~false

package main

func main() {
        var a struct {
                x int
                y float64
        }
        var b struct {
                x int
                y float64
        }
        println(a == b)

        a.x = 5
        b.x = 6

        println(a == b)

        a = b;

}
