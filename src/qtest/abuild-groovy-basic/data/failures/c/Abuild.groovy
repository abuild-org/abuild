abuild.setTarget('one', 'deps':'two')
abuild.setTarget('two', 'deps':'one')
abuild.setTarget('three', 'deps':'four')
abuild.prop['abuild.local-rules'] = ['local1.groovy']
