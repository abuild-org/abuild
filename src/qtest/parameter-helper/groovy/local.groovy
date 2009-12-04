import org.abuild.groovy.BuildFailure

def outer = 'variable defined outside'
def f = { it + 1 }
parameters {
    a.b.c.d = 'some value'
    x.y.z << 3
    x.y.z << 4
    z << 5
    z = 10
    z << 15
    q = 'r'
    assert [10, 15] == resolve('z')
    delete z
    p.q.r << resolve(a.b.c.d)
    def v = 'some other value'
    a.b.c.d = v
    scope = outer
    list << [1, 2] << resolve(x.y.z) << [5, [6], resolve(q)] << 7
    try
    {
        s.t.u = x.y.z
        assert false
    }
    catch (BuildFailure e)
    {
        assert e.message =~ /resolve/
    }
    s.t.u = resolve(x.y.z)
    w.w.w = resolve(n.u.l.l)
    something.'with-dashes' = f(12)
    xjc = ['a': 'b']
    xjc << ['c': 'd']
    to.be.deleted = 'can\'t see this'
    // get interface variable whose name is not a valid groovy variable
    some.value = resolve('interface-variable-name')
    // okay to use resolve on a variable too
    other.value = resolve(a.b.c.d)
    try
    {
        yet_another_value = other.ifc.variable
        assert false
    }
    catch (Exception e)
    {
        assert e.class.name == 'org.abuild.groovy.BuildFailure'
        assert e.message =~ /resolve/
    }
    yet_another_value = resolve(other.ifc.variable)
    // Append to list variable with initial value coming from
    // interface
    item.list.variable << 'd' << 'e' << 'f'
}
abuild.setParameter('other1', 'other1 value')
abuild.appendParameter('other2', 'other2 value')
abuild.deleteParameter('to.be.deleted')

def results = abuild.params.keySet().sort().collect {
    k -> [k, abuild.resolve(k)] }
assert [
    ['a.b.c.d', 'some other value'],
    ['abuild.localRules', 'local.groovy'],
    ['item.list.variable', ['a', 'b', 'c', 'd', 'e', 'f']],
    ['list', [1, 2, 3, 4, 5, 6, 'r', 7]],
    ['other.value', 'some other value'],
    ['other1', 'other1 value'],
    ['other2', ['other2 value']],
    ['p.q.r', ['some value']],
    ['q', 'r'],
    ['s.t.u', [3, 4]],
    ['scope', 'variable defined outside'],
    ['some.value', ['one', 'two', 'three']],
    ['something.with-dashes', 13],
    ['w.w.w', null],
    ['x.y.z', [3, 4]],
    ['xjc', [['a': 'b'], ['c': 'd']]],
    ['yet_another_value', 'stringval']
    ] == results

println "all assertions passed"
