parameters {
    java.jarName = 'b.jar'
    // groovy first so java classes can see groovy classes
    abuild.rules = ['groovy', 'java']
}
