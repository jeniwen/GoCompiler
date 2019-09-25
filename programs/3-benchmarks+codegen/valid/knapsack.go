package main

type knapsack struct {
        maxW      int
        currItems int
}

type weightarray [100]int
type valuearray [100]int //parallel arrays

var seed int = 317

type DPmatrix [][]int

func randint() int {
        seed = 214013*seed + 2531011
        return (seed >> 16) & 0x7FFF
}

func max(a, b int) int {
        //I know this is excessive just trying out switch statements
        switch a > b {
        case true:
                return a
        case false:
                return b
        }
        return a
}


func main() {
        var myKS knapsack
        var w weightarray
        var v valuearray
        var scale int = 1000
        var currHigh int = -1;

        myKS.maxW = 5000

        //Assigning random weights and values to the items
        for i := 0; i < len(w); i++ {
                seed = randint()
                w[i] = seed / scale
                seed = randint()
                v[i] = seed / scale
                println(w[i], v[i])
        }

        //Define m[i,w] to be max value that can be obtained with weight
        //less than or equal to w usint items up to i
        var m [100][5001]int
        for j := 0; j <= myKS.maxW; j++ {
                m[0][j] = 0
        }

        for i := 1; i < 100; i++ {
                for j := 0; j <= myKS.maxW; j++ {
                        if w[i] > j {
                                m[i][j] = m[i-1][j]
                        } else {
                                m[i][j] = max(m[i-1][j], m[i-1][j-w[i]]+v[i])
                        }
                        println("items:", i, "weight:", j, m[i][j])

                        if m[i][j]>currHigh {
                                currHigh = m[i][j]
                        }

                }
        }
        println("Highest possible value:", currHigh);

}
