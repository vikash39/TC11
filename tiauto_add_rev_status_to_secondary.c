/*******************************************************************************
File         : tiauto_add_rev_status_to_secondary.c

Description  : This server exit method create the item based on the input.
				This method is called from the Data Migration utility.

Input        : None

Output       : None

Author       : TCS

Revision History :
Date            Revision    Who                 Description
*******************************************************************************
30-Nov-2012    1.0        Dipak Naik			Initial Creation
*******************************************************************************/

#include <tiauto_server_exits.h>
#include <tiauto_defines.h>
#include <tiauto_utils.h>

extern int TAUTO_Add_Rev_Status_to_Secondary(void *return_value)
{

	int		iNoOfRelStatus		= 0;
	int		iFail				= 0;
	char   *classid_s			= NULL;
	tag_t	tItemRev			= NULLTAG;
	tag_t	tCostForm			= NULLTAG;
	tag_t	*ptReleaseStatus	= NULL;
	tag_t	object_classID		= NULLTAG;
	int retainReleaseDate		= 0;
	logical verdict = false;
	tag_t object_reldateID = NULLTAG;
	tag_t object_relStatListID = NULLTAG;
	date_t dateVal = NULLDATE;
	time_t rawtime;
	struct tm * timeinfo;
	logical	lValid_date=true;
	SYSTEMTIME str_t;
	time_t timeObj;
	 //get the input arguments

	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	iFail = USERARG_get_tag_argument(&tItemRev);
	iFail = USERARG_get_tag_argument(&tCostForm);
	iFail = USERARG_get_int_argument(&retainReleaseDate);

	//create the item based on the input data
	iFail = ITK_set_bypass(true);
	iFail = AOM_ask_value_tags(tItemRev,"release_status_list",&iNoOfRelStatus,&ptReleaseStatus);
	if(iNoOfRelStatus>=0 && ptReleaseStatus != NULL)
	{

		if(retainReleaseDate==0)
		{
			iFail = POM_class_of_instance(tCostForm, &object_classID);
			if ( iFail == ITK_ok && object_classID!=NULLTAG)
			{
				iFail = POM_name_of_class(object_classID,&classid_s);
			}
			if ( iFail == ITK_ok )
			{
				//iFail = AOM_ask_value_date(tItemRev,"date_released", &dateVal);
				printf(asctime(timeinfo));

				dateVal.day = timeinfo->tm_mday;
				dateVal.hour = timeinfo->tm_hour;
				dateVal.minute = timeinfo->tm_min;
				dateVal.month = timeinfo->tm_mon;
				dateVal.second = timeinfo->tm_sec;
				dateVal.year = timeinfo->tm_year+1900;

				printf(" IFail=%d",iFail);
				iFail = POM_attr_id_of_attr("date_released",classid_s,&object_reldateID);
				iFail = POM_attr_id_of_attr("release_status_list",classid_s,&object_relStatListID);

				if ( iFail == ITK_ok )
				{
					iFail = POM_is_loaded(tCostForm, &verdict);
				}
				if (verdict == TRUE && iFail == ITK_ok)
				{
					iFail=POM_unload_instances(1, &tCostForm);
				}
				//Modifying the values of the new form
				if (iFail == ITK_ok)
				{
					iFail = POM_load_instances_any_class(1, &tCostForm, POM_modify_lock);
				}
				iFail = POM_clear_attr	( 1, &tCostForm, object_relStatListID);
				iFail = EPM_add_release_status(ptReleaseStatus[0],1,&tCostForm,false);
				iFail = POM_unload_instances(1, &tCostForm);
				iFail = POM_load_instances_any_class(1, &tCostForm, POM_modify_lock);
				iFail = POM_set_attr_date(1, &tCostForm, object_reldateID, dateVal);

				if (iFail == ITK_ok)
				{
					iFail = POM_save_instances(1, &tCostForm, FALSE);
				}
			}
		}
		else
		{
			iFail = AOM_ask_value_date(tItemRev,"date_released", &dateVal);
			iFail = POM_class_of_instance(tCostForm, &object_classID);
			if ( iFail == ITK_ok && object_classID!=NULLTAG)
			{
				iFail = POM_name_of_class(object_classID,&classid_s);
			}
			if ( iFail == ITK_ok )
			{
				iFail = POM_attr_id_of_attr("date_released",classid_s,&object_reldateID);
				iFail = POM_attr_id_of_attr("release_status_list",classid_s,&object_relStatListID);
				if ( iFail == ITK_ok )
				{
					iFail = POM_is_loaded(tCostForm, &verdict);
				}
				if (verdict == TRUE && iFail == ITK_ok)
				{
					iFail=POM_unload_instances(1, &tCostForm);
				}
				//Modifying the values of the new form
				if (iFail == ITK_ok)
				{
					iFail = POM_load_instances_any_class(1, &tCostForm, POM_modify_lock);
				}
				iFail = POM_clear_attr	( 1, &tCostForm, object_relStatListID);
				iFail = EPM_add_release_status(ptReleaseStatus[0],1,&tCostForm,false);
				iFail = POM_unload_instances(1, &tCostForm);
				iFail = POM_load_instances_any_class(1, &tCostForm, POM_modify_lock);
				iFail = POM_set_attr_date(1, &tCostForm, object_reldateID, dateVal);

				if (iFail == ITK_ok)
				{
					iFail = POM_save_instances(1, &tCostForm, FALSE);
				}
			}
		}

	}
    EMH_clear_errors();
	SAFE_MEM_free(ptReleaseStatus);
	return iFail;
}
