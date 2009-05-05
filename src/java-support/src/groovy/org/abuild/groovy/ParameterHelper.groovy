package org.abuild.groovy

import org.abuild.groovy.Parameterized

class ParameterHelper
{
    private Parameterized _p
    private String _name

    public static createClosure(Parameterized p)
    {
        return {
            cl ->
            def old_d = cl.getDelegate()
            def old_r = cl.getResolveStrategy()
            cl.setDelegate(new ParameterHelper(p))
            cl.setResolveStrategy(Closure.DELEGATE_ONLY)
            cl()
            cl.setDelegate(old_d)
            cl.setResolveStrategy(old_r)
        }
    }

    private ParameterHelper(Parameterized p)
    {
        this._p = p
        this._name = ''
    }

    private ParameterHelper(Parameterized p, String name)
    {
        this._p = p
        this._name = name
    }

    private String child(String property)
    {
        (_name ? "${_name}." : '') + property
    }

    public Object get(String property)
    {
        return new ParameterHelper(this._p, child(property))
    }

    public void set(String property, Object value)
    {
        if (value instanceof ParameterHelper)
        {

            // Although it would be possible to avoid having to put
            // parameters in calls to resolve on the right hand side
            // of assignment or append operators just by resolving
            // them here automatically, there are all sorts of cases
            // where it doesn't work, such as when parameters are used
            // as values in map keys, passed as arguments to arbitrary
            // functions, or assigned to local variables.  It would be
            // possible to alleviate this to some extent by changing
            // the implementation of ParameterHelper to automatically
            // resolve parameter names to values a resolution is
            // available.  However, this would prevent simultaneous
            // use of a.b and a.b.c, as in abuild.classpath and
            // abuild.classpath.manifest.  To keep things simple and
            // avoid confusion, we force use of resolve in all cases.
            // That way, people get into the habit of using it and
            // won't be thrown off by having ParameterHelper objects
            // floating around in unexpected places.
            throw new BuildFailure(
                'abuild parameter names must be passed' +
                ' to resolve to be used on the right hand side of' +
                ' an assignment')
        }
        else
        {
            _p.setParameter(child(property), value)
        }
    }
    public ParameterHelper leftShift(Object value)
    {
        if (value instanceof List)
        {
            value.each { this << it }
        }
        else if (value instanceof ParameterHelper)
        {
            // See comment above.
            throw new BuildFailure(
                'abuild parameter names must be passed' +
                ' to resolve to be used on the right hand side of' +
                ' an append operator')
        }
        else
        {
            _p.appendParameter(_name, value)
        }
        return this
    }

    public void delete(String name)
    {
        _p.deleteParameter(name)
    }

    public void delete(ParameterHelper ph)
    {
        _p.deleteParameter(ph._name)
    }

    Object resolve(String name)
    {
        _p.resolve(name)
    }

    Object resolve(ParameterHelper ph)
    {
        resolve(ph._name)
    }
}
