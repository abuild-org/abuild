<?xml version="1.0"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
 <xsl:output method="text" standalone="no"/>
 <xsl:template match="/">
  <xsl:for-each select="requirements/reqtable/req">
   <xsl:value-of select="id"/>
   <xsl:text>&#x0a;</xsl:text>
  </xsl:for-each>
 </xsl:template>
</xsl:stylesheet>
