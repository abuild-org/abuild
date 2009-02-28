
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

println abuild.resolveVariable('potato.salad')
println abuild.resolveVariableAsString('qtest.export')
println abuild.resolveVariableAsString('VAR1')
println abuild.resolveVariableAsList('qtest.export').join(',')
println abuild.resolveVariableAsList('VAR1').join(',')
