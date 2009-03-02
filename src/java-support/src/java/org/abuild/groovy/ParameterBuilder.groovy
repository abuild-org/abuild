package org.abuild.groovy

class ParameterBuilder extends BuilderSupport
{
    Map<String, Object> parameters = [:]

    def createNode(name)
    {
        createNode(name, null, null)
    }

    def createNode(name, value)
    {
        createNode(name, null, value)
    }

    def createNode(name, Map attributes)
    {
        createNode(name, attributes, null)
    }

    def createNode(name, Map attributes, value)
    {
        String key = current

        if (key == null)
        {
            if (name == 'call')
            {
                key = ''
            }
            else
            {
                key = name
            }
        }
        else
        {
            key = (key ? "${key}." : "") + name
        }

        if ((value != null) || (attributes != null))
        {
            if (key == '')
            {
                throw new Exception("attempt to set empty parameter")
            }

            // Desired semantics: when we add exactly one thing, that
            // one thing should be the value.  Otherwise, the value
            // should be a list of everything we add.  When we add one
            // thing and that one thing is a list, treat it as if we
            // added the members of the list.  This is the only way to
            // be unambiguous.

            def toAdd

            if (value == null)
            {
                toAdd = [attributes]
            }
            else
            {
                if (value instanceof List)
                {
                    toAdd = value
                }
                else
                {
                    toAdd = [value]
                }
                if (attributes != null)
                {
                    toAdd << attributes
                }
            }

            assert toAdd instanceof List

            if (parameters.containsKey(key))
            {
                if (! (parameters[key] instanceof List))
                {
                    parameters[key] = [parameters[key]]
                }
                parameters[key].addAll(toAdd)
            }
            else if (toAdd.size == 1)
            {
                parameters[key] = toAdd[0]
            }
            else
            {
                parameters[key] = toAdd
            }
        }

        key
    }

    void setParent(parent, child)
    {
    }
}
