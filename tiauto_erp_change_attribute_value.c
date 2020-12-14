/*******************************************************************************
File         : tiauto_erp_change_attribute_value.c

Description  : This server exit method create the item based on the input.
				This method is called from the Data Migration utility.

Input        : None
                        
Output       : None

Author       : TCS

Revision History :
Date            Revision    Who                 Description
*******************************************************************************
22-Apr-2014    1.0        Manik					Initial Creation
*******************************************************************************/

#include <tiauto_server_exits.h>
#include <tiauto_defines.h>
#include <tiauto_utils.h>

extern int TAUTO_ERP_Change_Attribute_Value(void *return_value)
{
	tag_t tObject		= NULLTAG;
	tag_t tClass		= NULLTAG;
	tag_t tObjName		= NULLTAG;
	char *pcAttrName	= NULL;
	char *pcAttrValue	= NULL;
	char *pcClassName	= NULL;
	int iFail = ITK_ok;
	logical verdict		= false;

	iFail = USERARG_get_tag_argument(&tObject);
	if(iFail==ITK_ok)
		iFail = USERARG_get_string_argument(&pcAttrName);
	if(iFail==ITK_ok)
		iFail = USERARG_get_string_argument(&pcAttrValue);
	if(iFail == ITK_ok && tObject!=NULLTAG && pcAttrName!=NULL && pcAttrValue!=NULL)
	{
		iFail = POM_is_loaded(tObject, &verdict);
		if (verdict == TRUE && iFail == ITK_ok)
		{
			iFail = POM_unload_instances(1, &tObject);
		}
		iFail = POM_load_instances_any_class(1, &tObject, POM_modify_lock);
		iFail = POM_class_of_instance (tObject, &tClass);
		iFail = POM_name_of_class (tClass, &pcClassName);
		iFail = POM_attr_id_of_attr(pcAttrName, pcClassName, &tObjName);
		iFail = POM_set_attr_string(1, &tObject, tObjName, pcAttrValue );

		//iFail = AOM_set_value_string(tObject,pcAttrName,pcAttrValue);
		if(iFail==ITK_ok)
			iFail = POM_save_instances(1, &tObject, FALSE);
		iFail = POM_refresh_instances_any_class(1, &tObject, POM_no_lock);
	}
	return iFail;
}