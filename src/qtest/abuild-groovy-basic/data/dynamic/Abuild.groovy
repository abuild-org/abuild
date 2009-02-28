parameters {
    abuild {
        rules('empty')
    }
    qtest {
        export('VAR1')
        export('VAR2')
        export('potato')
        export('spackle')
        export('truth1')
    }
    VAR1('first value')
    VAR2('second value')
}

println abuild.resolve('potato.salad')
println abuild.resolveAsString('VAR1')
println abuild.resolveAsList('qtest.export').join(',')
println abuild.resolveAsList('VAR1').join(',')
