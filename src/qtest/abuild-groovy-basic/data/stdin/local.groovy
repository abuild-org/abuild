abuild.configureTarget('all') {
    def inx = new BufferedReader(new InputStreamReader(System.in))
    println 'input: ' + inx.readLine()
}
