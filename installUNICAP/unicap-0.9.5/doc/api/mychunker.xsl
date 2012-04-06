<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns:exsl="http://exslt.org/common"
		version="1.0"
                extension-element-prefixes="exsl">

  <xsl:template name="write-chunk">
    <xsl:param name="filename"/>
    <xsl:param name="content"/>
    <xsl:choose>
      <xsl:when test="element-available('exsl:document')">
	<exsl:document href="{$filename}"
		       method="html"
		       encoding="UTF-8"
		       doctype-public="-//W3C//DTD XHTML 1.0 Strict//EN"
		       doctype-system="http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
	  <xsl:copy-of select="$content"/>
	</exsl:document>
      </xsl:when>
      <xsl:otherwise>
	<xsl:message>Error: exsl:document not available! :-(</xsl:message>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>
</xsl:stylesheet>