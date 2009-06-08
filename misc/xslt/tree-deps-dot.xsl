<?xml version="1.0"?>

<!--

Create a graph for dot (graphviz) showing trees and their declared
dependencies.  Reads the output of abuild ‐‐dump-data as input.
Example usage:

abuild ‐‐dump-data > deps.xml
xsltproc tree-deps-dot.xsl deps.xml > deps.dot
dot -Tpng -odeps.png deps.dot

(Note: two dashes above should be real ASCII -'s, not ‐ (U+2010).  The
latter is used only because we can't include two dashes in an XML comment.)

-->

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

 <xsl:output method="text" standalone="no"/>

 <xsl:template match="/">
  <xsl:text>digraph deps {&#x0a;</xsl:text>
  <xsl:for-each select="//forest[last()]">
   <xsl:for-each select="build-tree">
    <xsl:variable name="tree" select="@name"/>
    <xsl:text>"</xsl:text>
    <xsl:value-of select="$tree"/>
    <xsl:text>"&#x0a;</xsl:text>
    <xsl:for-each select="declared-tree-dependencies/tree-dependency">
     <xsl:text>"</xsl:text>
     <xsl:value-of select="$tree"/>
     <xsl:text>" -> "</xsl:text>
     <xsl:value-of select="@name"/>
     <xsl:text>"&#x0a;</xsl:text>
    </xsl:for-each>
   </xsl:for-each>
  </xsl:for-each>
  <xsl:text>}&#x0a;</xsl:text>
 </xsl:template>

</xsl:stylesheet>
