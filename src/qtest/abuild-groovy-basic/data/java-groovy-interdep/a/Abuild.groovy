parameters {
    java.jarName = 'a.jar'
    // java first so groovy classes can see java classes
    abuild.rules = ['java', 'groovy']
}
