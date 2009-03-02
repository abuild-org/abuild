
import org.abuild.groovy.ParameterBuilder

def builder = new ParameterBuilder()
builder {
    a(1)
    a([2])
    a([3, 4])
    b([1])
    b(2)
    b([3, 4])
    c(1)
    d([1])
    e(1, 'a':'b', 'q':'r')
    e('c':'d')
    f('e':'f')
    g(['g':'h'])
    h([['i':'j']])
    i([[['k':'l']]])
    b([[5]]) {
        x(1) {
            y()
        }
        y(1)
        y(2) {
            z('x')
        }
    }
    j([[1]])
}

assert [1, 2, 3, 4] == builder.parameters['a']
assert [1, 2, 3, 4, [5]] == builder.parameters['b']
assert 1 == builder.parameters['c']
assert 1 == builder.parameters['d']
assert [1, ['a':'b', 'q':'r'], ['c':'d']] == builder.parameters['e']
assert ['e':'f'] == builder.parameters['f']
assert ['g':'h'] == builder.parameters['g']
assert ['i':'j'] == builder.parameters['h']
assert [['k':'l']] == builder.parameters['i']
assert 1 == builder.parameters['b.x']
assert ! builder.parameters.containsKey('b.x.y')
assert [1, 2] == builder.parameters['b.y']
assert 'x' == builder.parameters['b.y.z']
assert [1] == builder.parameters['j']

println "all assertions passed"
