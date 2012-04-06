<xsl:stylesheet version = '1.0' xmlns:xsl='http://www.w3.org/1999/XSL/Transform'>

  <xsl:import href="mychunker.xsl"/>

  <!-- <xsl:output method="html" 
	      doctype-system="http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd" 
	      doctype-public="-//W3C//DTD XHTML 1.0 Strict//EN" /> -->

  
  <xsl:template match="/api">
    <xsl:for-each select="module">
      <xsl:variable name="c_module"><xsl:value-of select="@name"/></xsl:variable>

      <xsl:variable name="mod_content">
	<xsl:apply-templates select="module-page">
	  <xsl:with-param name="c_module" select="$c_module"/>
	</xsl:apply-templates>
      </xsl:variable>
      <xsl:variable name="mod_filename">mod_<xsl:value-of select="@name"/>.html</xsl:variable>
      <xsl:message> -> <xsl:value-of select="$mod_filename"/></xsl:message>
      <xsl:call-template name="write-chunk">
	<xsl:with-param name="filename" select="$mod_filename"/>
	<xsl:with-param name="content" select="$mod_content"/>
      </xsl:call-template>
	

      <xsl:choose>
	<xsl:when test="class">
	  <xsl:for-each select="class">

	    <xsl:variable name="c_class"><xsl:value-of select="@name"/></xsl:variable>

	    <xsl:variable name="class_content">
	      <xsl:apply-templates select="class-page">
		<xsl:with-param name="c_module" select="$c_module"/>
		<xsl:with-param name="c_class" select="$c_class"/>
	      </xsl:apply-templates>
	    </xsl:variable>
	    <xsl:variable name="class_filename">c_<xsl:value-of select="@name"/>.html</xsl:variable>
	    <xsl:message>  -> <xsl:value-of select="$class_filename"/></xsl:message>
	    <xsl:call-template name="write-chunk">
	      <xsl:with-param name="filename" select="$class_filename"/>
	      <xsl:with-param name="content" select="$class_content"/>
	    </xsl:call-template>

	    <xsl:call-template name="process-api-pages">
	      <xsl:with-param name="c_module" select="$c_module"/>
	      <xsl:with-param name="c_class" select="$c_class"/>
	    </xsl:call-template>
	  </xsl:for-each>
	</xsl:when>
	<xsl:otherwise>
	  <xsl:call-template name="process-api-pages">
	    <xsl:with-param name="c_module" select="$c_module"/>
	  </xsl:call-template>
	</xsl:otherwise>
      </xsl:choose>
    </xsl:for-each>
  </xsl:template>

  <xsl:template name="process-api-pages">
    <xsl:param name="c_module"/>
    <xsl:param name="c_class"/>
    <xsl:for-each select="api-page">
      <xsl:variable name="content">
	<xsl:apply-templates select=".">
	  <xsl:with-param name="c_module" select="$c_module"/>
	  <xsl:with-param name="c_class" select="$c_class"/>
	</xsl:apply-templates>

      </xsl:variable>
      <xsl:variable name="filename"><xsl:call-template name="get-name"/>.html</xsl:variable>
      <xsl:message>   -> <xsl:value-of select="$filename"/></xsl:message>
      <xsl:call-template name="write-chunk">
	<xsl:with-param name="filename" select="$filename"/>
	<xsl:with-param name="content" select="$content"/>
      </xsl:call-template>
    </xsl:for-each>
  </xsl:template>

  <xsl:template match="module-page">
    <xsl:param name="c_module"/>
    <xsl:param name="c_class"/>    
    <xsl:call-template name="api-page">
      <xsl:with-param name="c_module" select="$c_module"/>
      <xsl:with-param name="c_class" select="$c_class"/>
    </xsl:call-template>
  </xsl:template>

  <xsl:template match="class-page">
    <xsl:param name="c_module"/>
    <xsl:param name="c_class"/>
    <xsl:call-template name="api-page">
      <xsl:with-param name="c_module" select="$c_module"/>
      <xsl:with-param name="c_class" select="$c_class"/>
    </xsl:call-template>
  </xsl:template>

  <xsl:template match="api-page" name="api-page">
    <xsl:param name="c_module"/>
    <xsl:param name="c_class"/>
    <html>
    <head>
      <xsl:call-template name="make-title"/>
      <meta http-equiv="content-type" content="text/html; charset=UTF-8" />
      <meta name="description" content="unicap - The uniform API for image acquisition devices"/>
      <meta name="keywords" content="UniCap, linux, v4l, video4linux, image, capture, acquisition, FireWire, 1394, usb, camera, grabber, converter, video-to-firewire, video-to-usb, dfg1394"/>
      <meta name="robots" content="index"/>  
      <link href="gen.css" rel="stylesheet" type="text/css" media="all" />
      <link rel="shortcut icon" href="favicon.ico"/>
    </head>
    <body>
      <table width="100%" cellpadding="0" cellspacing="2" border="0" bordercolor="#FFFFFF">
	<!-- ******************* header ***************************** -->
	<tr align="left">
	  <td colspan="2">
	    <p class="logo">
	      <a href="http://unicap-imaging.org/" title="unicap - The uniform API for image acquisition devices">
		<img src="images/logo_unicap.gif" width="135" height="50" border="0" 
		     alt="unicap - The uniform API for image acquisition devices"/>
	      </a>
	    </p>
	  </td>
	</tr>
	
	<tr bgcolor="#EEEEEE" height="30">
	  <td>&#160;</td>
	  <td align="right">
	    <a href="http://sourceforge.net/export/rss2_project.php?group_id=104946" title="SourceForge RSS feed">
	      <img src="images/icon_xml.gif" width="36" height="14" border="0" alt="SourceForge RSS feed"/>
	      </a>&#160;&#160;&#160;
	  </td>
	</tr>
	
	<!-- ******************** navigation ****************************** -->
	<tr valign="top">
	  <td width="250" bgcolor="#EEEEEE">
	    <span class="menu">
	      <xsl:call-template name="make-navbar">
		<xsl:with-param name="c_module" select="$c_module"/>
		<xsl:with-param name="c_class" select="$c_class"/>
	      </xsl:call-template>
	      <p>&#160;</p>
	    </span>
	  </td>
	  <td>
	    <!-- ******************** content ****************************** -->
	    <xsl:apply-templates/>
	  </td>
	</tr>
	<!-- ******************* footer ***************************** -->
	<tr bgcolor="#EEEEEE" height="30">
	  <td valign="middle" height="40">
	    <xsl:text>&#160;&#160;</xsl:text>
	  </td>
	  <td>
	    <p class="copyright">
	      <xsl:text>&#169; 2006 Arne Caspari. All rights reserved.</xsl:text>
	    </p>
	  </td>
	</tr>
      </table>
    </body>
    </html>

  </xsl:template>

  <xsl:template name="make-title">
    <title><xsl:call-template name="get-name"/></title>
  </xsl:template>

  <xsl:template name="get-name">
    <xsl:choose>
      <xsl:when test="title">
	<xsl:value-of select="title"/>
      </xsl:when>
      <xsl:when test="function">
	<xsl:value-of select="function/function-synopsis/prototype/function-name/"/>
      </xsl:when>
      <xsl:when test="type">
	<xsl:value-of select="type/type-name/"/>
      </xsl:when>
      <xsl:otherwise>
	<xsl:message>Wrong node, not api-page?</xsl:message>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="html-code">
    <xsl:copy-of select="./*"/>
  </xsl:template>

  <xsl:template match="title">
  </xsl:template>

  <xsl:template name="make-navbar">
    <xsl:param name="c_module"/>
    <xsl:param name="c_class"/>    
    <p>
      <xsl:for-each select="/api/module">
	<a>
	  <xsl:attribute name="href">mod_<xsl:value-of select="@name"/>.html</xsl:attribute>
	  <xsl:attribute name="title">mod_<xsl:value-of select="@name"/>.html</xsl:attribute>
	  <xsl:value-of select="@name"/>
	</a>
	<br/>
	<xsl:if test="@name=$c_module">
	  <xsl:choose>
	    <xsl:when test="class">
	      <xsl:for-each select="class">
		&#160;
		<a>
		  <xsl:attribute name="href">c_<xsl:value-of select="@name"/>.html</xsl:attribute>
		  <xsl:attribute name="title">c_<xsl:value-of select="@name"/>.html</xsl:attribute>
		  <xsl:value-of select="@name"/>
		</a>
		
		<br/>
		<xsl:if test="@name=$c_class">
		  <xsl:for-each select="api-page">
		    &#160;&#160;&#160;
		    <a>
		    <xsl:attribute name="href"><xsl:call-template name="get-name"/>.html</xsl:attribute>
		    <xsl:attribute name="title"><xsl:call-template name="get-name"/>.html</xsl:attribute>
		    <xsl:call-template name="get-name"/>
		    </a>&#160;&#160;
		    <br/>
		  </xsl:for-each>
		</xsl:if>
	      </xsl:for-each>
	    </xsl:when>
	    <xsl:otherwise>
	      <xsl:for-each select="api-page">
		&#160;<a>
		<xsl:attribute name="href"><xsl:call-template name="get-name"/>.html</xsl:attribute>
		<xsl:attribute name="title"><xsl:call-template name="get-name"/>.html</xsl:attribute>
		<xsl:call-template name="get-name"/>
		</a>&#160;&#160;
		<br/>
	      </xsl:for-each>
	    </xsl:otherwise>
	  </xsl:choose>
	</xsl:if>
      </xsl:for-each>
    </p>
  </xsl:template>


  <xsl:template match="function">
    <p>
      <xsl:apply-templates select="function-synopsis"/>
    </p>
    <p>
      <xsl:apply-templates select="function-description"/>
    </p>
    <p>
      <xsl:apply-templates select="function-parameters"/>
    </p>
  </xsl:template>

  <xsl:template match="function-synopsis">
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="prototype">
    <span class="funcproto">
      <table border="0">
	<tr>
	  <td valign="top">
	    <span class="returnval">
	      <xsl:apply-templates select="return-value"/>
	    </span>
	  </td>
	  <td valign="top">
	    <span class="funcname">
	      <xsl:value-of select="function-name"/>
	    </span>
	    (
	  </td>
	  <td>
	    <xsl:for-each select="parameter">
	      <span class="funcparamtype">
		<xsl:value-of select="parameter-type"/>
	      </span>
	      <span class="funcparamname">
		<xsl:value-of select="parameter-name"/>
	      </span>
	      <xsl:if test="not(position()=last())">
		<xsl:text>,&#160;</xsl:text><br/>
	      </xsl:if>
	    </xsl:for-each>
	    );
	  </td>
	</tr>
      </table>
    </span>
    <br/>
  </xsl:template>

  <xsl:template match="function-description">
    <span class="funcdesc">
      <xsl:apply-templates/>
    </span>
  </xsl:template>

  <xsl:template match="function-parameters">
    <table border="0">

      <xsl:for-each select="parameter-description">
      
	<tr>
	  <td align="right" valign="top" width="150px">
	    <div class="paramname">
	      <xsl:value-of select="parameter-name"/>:
	    </div>
	  </td>
	  <td align="left">
	    <span class="paramdesc">
	      <xsl:apply-templates select="parameter-text"/>
	    </span>
	  </td>
	</tr>

      </xsl:for-each>

    </table>
  </xsl:template>

  <xsl:template match="type">
    <p>
      <span class="typedef">
	<xsl:choose>
	  <xsl:when test="type-field">
	    <xsl:call-template name="make-type-struct"/>
	  </xsl:when>
	  <xsl:when test="enum">
	    <xsl:call-template name="make-type-enum"/>
	  </xsl:when>
	</xsl:choose>
      </span>
    </p>
    <p>
      <xsl:choose>
	<xsl:when test="type-field">
	  <xsl:call-template name="make-type-struct-desc"/>
	</xsl:when>
	<xsl:when test="enum">
	  <xsl:call-template name="make-type-enum-desc"/>
	</xsl:when>
      </xsl:choose>
    </p>      
  </xsl:template>

  <xsl:template name="make-type-struct">
    <table border="0">
      <tr>
	<td valign="top">
	  typedef struct {
	</td>
	<td valign="top">
	  <table border="0" top-margin="0">
	    <tr>
	      <td align="right" valign="top">
		<xsl:for-each select="type-field">
		  <span class="fieldtype"><xsl:value-of select="field-type"/></span><br/>
		</xsl:for-each>
	      </td>
	      <td align="left" valign="top">
		<xsl:for-each select="type-field">
		  <xsl:value-of select="field-name"/>
		  <xsl:if test="number(field-array-size)">
		    [<xsl:value-of select="field-array-size"/>]
		  </xsl:if>
		  ;<br/>
		</xsl:for-each>
	      </td>
	    </tr>
	  </table>
	  }<xsl:value-of select="type-name"/>;
	  
	</td>
      </tr>
    </table>
  </xsl:template>

  <xsl:template name="make-type-struct-desc">
    <table border="0">
      <xsl:for-each select="type-field">
	<tr>
	  <td valign="top" align="right">
	    <span class="fieldname"><xsl:apply-templates select="field-name"/></span>:
	  </td>
	  <td>
	    <xsl:apply-templates select="field-description"/>
	  </td>
	</tr>
      </xsl:for-each>
    </table>
  </xsl:template>
  
  <xsl:template name="make-type-enum">
    <table border="0">
      <tr>
	<td valign="top">
	  typedef enum {
	</td>
	<td>
	  <xsl:for-each select="enum/enum-element">
	    <span class="enumname"><xsl:value-of select="enum-element-name"/></span>
	    <xsl:if test="@value">
	      = <xsl:value-of select="@name"/>
	    </xsl:if>
	    <xsl:if test="not(position()=last())">
	      ,<br/>
	    </xsl:if>
	  </xsl:for-each>
	</td>
      </tr>
    </table>
  </xsl:template>

  <xsl:template name="make-type-enum-desc">
    <table border="0">
      <xsl:for-each select="enum/enum-element">
	<tr>
	  <td valign="top" align="right">
	    <xsl:apply-templates select="enum-element-name"/>:
	  </td>
	  <td>
	    <xsl:apply-templates select="enum-element-description"/>
	  </td>
	</tr>
      </xsl:for-each>
    </table>
  </xsl:template>

  <xsl:template name="get-typename">
    <xsl:param name="rawname"/>
    <xsl:choose>
      <xsl:when test="not(strlen(substring-before($rawname,' '))=0)"><xsl:value-of select="substring-before($rawname,' ')"/></xsl:when>
      <xsl:otherwise><xsl:value-of select="$rawname"/></xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="get-api-link">
    <xsl:param name="pagename"/>
      <xsl:for-each select="/api">
	<xsl:for-each select="module">
	  <xsl:choose>
	    <xsl:when test="class">
	      <xsl:for-each select="class">
		<xsl:for-each select="class-page">
		  <xsl:variable name="name"><xsl:call-template name="get-name"/></xsl:variable>
		  <xsl:if test="$name=$pagename">c_<xsl:value-of select="$name"/>.html</xsl:if>		  
		</xsl:for-each>
		<xsl:for-each select="api-page">
		  <xsl:variable name="name"><xsl:call-template name="get-name"/></xsl:variable>
		  <xsl:if test="$name=$pagename"><xsl:value-of select="$pagename"/>.html</xsl:if>
		</xsl:for-each>
	      </xsl:for-each>
	    </xsl:when>
	    <xsl:otherwise>
	      <xsl:for-each select="api-page">
		<xsl:variable name="name"><xsl:call-template name="get-name"/></xsl:variable>
		<xsl:if test="$name=$pagename"><xsl:value-of select="$pagename"/>.html</xsl:if>
	      </xsl:for-each>
	    </xsl:otherwise>
	  </xsl:choose>
	</xsl:for-each>
      </xsl:for-each>
  </xsl:template>

  <xsl:template match="return-value">
    <a>
      <xsl:attribute name="href"><xsl:call-template name="get-api-link"><xsl:with-param name="pagename"><xsl:value-of select="."/></xsl:with-param></xsl:call-template></xsl:attribute>
      <xsl:value-of select="."/>
    </a>
  </xsl:template>

  <xsl:template match="api-link">
    <a>
      <xsl:attribute name="href"><xsl:call-template name="get-api-link"><xsl:with-param name="pagename"><xsl:value-of select="."/></xsl:with-param></xsl:call-template></xsl:attribute>
      <xsl:value-of select="."/>
    </a>
  </xsl:template>

  <xsl:template match="param-name">
    <span class="paramname">
      <xsl:copy-of select="."/>
    </span>
  </xsl:template>

  <xsl:template match="p">
    <p><xsl:apply-templates/></p>
  </xsl:template>

  <xsl:template match="field-name">
    <span class="fieldname"><xsl:copy-of select="."/></span>
  </xsl:template>
	
</xsl:stylesheet>
