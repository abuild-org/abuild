abuild.addTargetClosure('echo') {
    ant.echo("This is a message from the echoer plugin.")
    ant.echo("The value of echo.message is " + abuild.resolve('echo.message'))
}
abuild.addTargetDependencies('all', 'echo')
