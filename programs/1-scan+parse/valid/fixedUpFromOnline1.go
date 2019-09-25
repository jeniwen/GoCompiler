package main // Figured golite is just a subset of go, so we should just be able to test this some fixed up go code from google

func main() {

    i := 1
    for i <= 3 {
        println(i)
        i = i + 1
    }

    for j := 7; j <= 9; j++ {
        println(j)
    }

    for {
        println("loop")
        break
    }

    for n := 0; n <= 5; n++ {
        if n%2 == 0 {
            continue
        }
        println(n)
    }
}
