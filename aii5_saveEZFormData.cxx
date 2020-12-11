/******************************************************************************
*------------------------------------------------------------------------------
* Filename			:   aii5_saveEZFormData.cxx
* Description		:   EZ Form SOA implementation. EZ Form Save operation.
*						
* Module  		    :   libsoaaltec.dll
*
* ENVIRONMENT		:    C, C++, ITK
*
* History
*------------------------------------------------------------------------------
* Date(MM-DD-YYY)	Name							Description of Change
* 04-20-2020      	Dipen Prajapati	        		Initial Code
*
*****************************************************************************/


#include <unidefs.h>
#if defined(SUN)
#include <unistd.h>
#endif
#include "aii5_saveEZFormData.h"
#include <algorithm>

using namespace Aii5::Soa::Altec::_2017_11;

string get_zoom_template_internal_value(string strDisplayValue)
{
	int     iFail          = ITK_ok;
	string strLOV="";
	tag_t tagLOV=NULLTAG;
	tag_t *tagLOVS=NULL;
	int nLOVs=0;
	char **charDisplayValues;
	char **charValues;
	LOV_usage_t lov_usage ;
	int iNoOfLOVValue=0;

	ITK_CALL(iFail,LOV_find (Aii4_ZoomTempOrdered_LOV,&nLOVs,&tagLOVS));
	if(nLOVs>0)
	{
		tagLOV=tagLOVS[0];
	}
	ITK_free_memory(tagLOVS);
	ITK_CALL(iFail,LOV_ask_values_display_string(tagLOV,&lov_usage,&iNoOfLOVValue,&charDisplayValues,&charValues));
	for(int i=0;i<iNoOfLOVValue;i++)
	{
		if(tc_strcmp(strDisplayValue.c_str(),charDisplayValues[i])==0)
		{
			//Found
			strLOV=charValues[i];
			break;
		}
	}
	return strLOV;
}

int delete_relation_primary_to_secondary ( 
	                                       tag_t primary_object  , 
										   char *relation_name ,
										   Aii5::Soa::Altec::_2017_11::AltecTCExtension::SaveEZFormResponse &output
										   )
{
	int     iFail          = ITK_ok;
	tag_t   relation_type  = NULLTAG;  
	int     relation_count = 0;  
    tag_t   *relation_list = NULL;

	 ITK_CALL(iFail,GRM_find_relation_type  ( relation_name , &relation_type ) );
	 ITK_CALL(iFail,GRM_list_relations  ( primary_object, NULLTAG , relation_type, NULLTAG , &relation_count , &relation_list )); 
    if( relation_count == 1 )
	{
		ITK_CALL2(iFail, GRM_delete_relation  ( relation_list[ 0 ] ),output,FAILEDTOREVISE) ;
	}
	ITK_free_memory( relation_list );
	return iFail;
}

int create_relation_primary_to_secondary ( 
	                                       tag_t primary_object , 
										   tag_t secondary_object , 
										   char *relation_name ,
										    Aii5::Soa::Altec::_2017_11::AltecTCExtension::SaveEZFormResponse &output
										   )
{
	int     iFail         = ITK_ok;
	tag_t   relation_type = NULLTAG;
	tag_t   relation      = NULLTAG;  
//	ITK_CALL(iFail,AOM_refresh(primary_object, true));
//	ITK_CALL(iFail,AOM_refresh(secondary_object, true));
	ITK_CALL(iFail,GRM_find_relation_type  ( relation_name , &relation_type )) ;
	ITK_CALL(iFail,GRM_create_relation  ( primary_object , secondary_object , relation_type , NULLTAG , &relation )); 
	ITK_CALL(iFail,GRM_save_relation  ( relation )) ;
//	ITK_CALL(iFail,AOM_refresh(primary_object, false));
//	ITK_CALL(iFail,AOM_refresh(secondary_object, false));
	// Release lock on relation too!
	ITK_CALL(iFail,AOM_refresh(relation, false));
	return iFail;
}

logical is_date_pattern_matches( 
								tag_t  mbom_rev 
								)
{	
	int       length     = 0;
	logical   result     = false; 
	char      *sent_date = NULL;
	int iFail=ITK_ok;

    ITK_CALL(iFail, AOM_ask_value_string  ( mbom_rev , "aii4_EBSBOMNo" , &sent_date ));
	if( sent_date != NULL )
	{
		length = strlen( sent_date );
		if(length>=4)
		{
			if( sent_date[ length - 4 ] == '_' )
			{
				result = true;
			}
		}
		
	}
	ITK_free_memory( sent_date );
	return result;
}

int create_relation_primary_to_secondary ( 
	                                       tag_t primary_object , 
										   tag_t secondary_object , 
										   string relation_name ,
										    Aii5::Soa::Altec::_2017_11::AltecTCExtension::SaveEZFormResponse &output
										   )
{
	int     iFail         = ITK_ok;
	tag_t   relation_type = NULLTAG;
	tag_t   relation      = NULLTAG;  
	ITK_CALL2(iFail,AOM_refresh(primary_object, true),output,FAILETOCREATERELATION);
	//ITK_CALL(iFail,AOM_refresh(secondary_object, true));
	ITK_CALL2(iFail,GRM_find_relation_type  ( relation_name.c_str() , &relation_type ),output,FAILETOCREATERELATION) ;
	ITK_CALL2(iFail, GRM_create_relation  ( primary_object , secondary_object , relation_type , NULLTAG , &relation ),output,FAILETOCREATERELATION); 
	ITK_CALL2(iFail, GRM_save_relation  ( relation ),output,FAILETOCREATERELATION) ;
	ITK_CALL2(iFail,AOM_refresh(primary_object, false),output,FAILETOCREATERELATION);
	//ITK_CALL(iFail,AOM_refresh(secondary_object, false));
	// Release lock on relation too!
	ITK_CALL2(iFail,AOM_refresh(relation, false),output,FAILETOCREATERELATION);
	return iFail;
}

int copy_mbomrev_to_mbom_bvr(  tag_t   mbom_item_rev ,  tag_t   copy_mbom_rev,  Aii5::Soa::Altec::_2017_11::AltecTCExtension::SaveEZFormResponse &output)
{
	int     iFail              = 0;
	int     i                  = 0;
	int     part_child_count   = 0;
	tag_t   mbom_window        = NULLTAG;   
    tag_t   mbom_top_bom_line  = NULLTAG;
	tag_t   mbom_child_line   = NULLTAG;
	tag_t t_RevisionRule=NULLTAG;

	TC_write_syslog("In copy_mbomrev_to_mbom_bvr\n"); 
	ITK_CALL(iFail,AOM_refresh(mbom_item_rev,true));
	ITK_CALL(iFail, BOM_create_window ( &mbom_window ));
	ITK_CALL(iFail,CFM_find("Precise Only",&t_RevisionRule)); // Need to get it from Preference.
	ITK_CALL(iFail,BOM_set_window_config_rule(mbom_window,t_RevisionRule));

	ITK_CALL(iFail, BOM_set_window_top_line  ( mbom_window , NULLTAG , mbom_item_rev , NULLTAG , &mbom_top_bom_line ));
	if(iFail!=ITK_ok)
	{
				char *err_string=NULL;
				EMH_ask_error_text (iFail, &err_string); 
				if(err_string!=NULL)
				{
					output.isSuccess=false;
					output.strErrorList.push_back(err_string);
				}
				ITK_free_memory(err_string);
	}

	ITK_CALL2(iFail,BOM_line_add  ( mbom_top_bom_line , NULLTAG, copy_mbom_rev, NULLTAG, &mbom_child_line ),output,FAILEDTOADDBOMLINE);
	if(iFail!=ITK_ok)
	{
		char *err_string=NULL;
		EMH_ask_error_text (iFail, &err_string); 
		if(err_string!=NULL)
		{
			output.isSuccess=false;
			output.strErrorList.push_back(err_string);
		}
		ITK_free_memory(err_string);
	}
		// add seq number for top line of common bom
	TC_write_syslog("line tag %d\n", mbom_child_line); 
	if (mbom_child_line != NULLTAG) 
	{
		char initial_seq[10] = "1"; 
		int attr_num = 0;		
		ITK_CALL(iFail,BOM_line_look_up_attribute( bomAttr_occSeqNo , &attr_num ));
		ITK_CALL(iFail, BOM_line_set_attribute_string  ( mbom_child_line , attr_num , initial_seq ));	
		TC_write_syslog("set seq line %s\n", initial_seq); 
	}

	ITK_CALL2(iFail, BOM_save_window ( mbom_window ),output,FAILETOSAVEBOM);
	if(iFail!=ITK_ok)
	{
		char *err_string=NULL;
		EMH_ask_error_text (iFail, &err_string); 
		if(err_string!=NULL)
		{
			output.isSuccess=false;
			output.strErrorList.push_back(err_string);
		}
		ITK_free_memory(err_string);
	}
	ITK_CALL(iFail, BOM_close_window ( mbom_window ));
	ITK_CALL(iFail,AOM_save(mbom_item_rev));
	ITK_CALL(iFail,AOM_refresh(mbom_item_rev,false));
	
	return iFail;
}

/****************************************************************************/
//
//  Function Name:   GetItemBOMView
//
//  Description:     Check if Item has BOM view , if not return null
//                   return BOMView tag.
//
/*
//  Parameters:     tag_t itemTag,
					tag_t inputBVTypeTag,
					tag_t* bvTag
//                  
//                         
//
//  Return Value:    true - on Success
//                   false - on Failure
//
//  Special Logic Notes:  None
//
/* History
*------------------------------------------------------------------------------
* Date				Name							Description of Change
* 03-05-2020      	Dipen Prajapati	        		Initial Code
*
/****************************************************************************/
int GetItemBOMView1(tag_t itemTag, tag_t inputBVTypeTag, tag_t* bvTag)
{
	int   iFail      = ITK_ok;
	tag_t bvTypeTag = NULLTAG;
	tag_t* itemBVs = NULL;
	int count = 0;
	try
	{
		*bvTag = NULLTAG;

		// get list of BVs
		ITK_CALL(iFail,ITEM_list_bom_views(itemTag, &count, &itemBVs));

		for(int i = 0; i < count; i++)
		{
			bvTypeTag = NULLTAG;
			ITK_CALL(iFail,PS_ask_bom_view_type(itemBVs[i], &bvTypeTag));
			if(inputBVTypeTag == bvTypeTag)
			{
				*bvTag = itemBVs[i];
				break;
			}
		}
		MEM_free(itemBVs);
	}
	catch(...)
	{
		MEM_free(itemBVs);
		
	}
	return iFail;
}

/****************************************************************************/
//
//  Function Name:   CheckBVRExist
//
//  Description:     This method check if selected object has BVR , if not it will create new BVR , It will also check if Item has BOMView
//					 and create BOMView on Item if not exist.
//                  
//
/*
//  Parameters:     tag_t tagMBOMRev,
					Aii5::Soa::Altec::_2017_11::AltecTCExtension::SaveBOMFormResponse &output
//					
//					                  
//                         
//
//  Return Value:    tag_t Return selected object tag.
//                   false - on Failure
//
//  Special Logic Notes:  None
//
/* History
*------------------------------------------------------------------------------
* Date				Name							Description of Change
* 04-21-2020      	Dipen Prajapati	        		Initial Code
*
/****************************************************************************/

bool CheckBVRExist1(tag_t itemTag, tag_t itemRevTag,tag_t* bvrTag,Aii5::Soa::Altec::_2017_11::AltecTCExtension::SaveEZFormResponse &output)
{
	bool	response	=	true;
	int		iFail		=	ITK_ok;
	tag_t	bomview_tag =	NULLTAG;
	
	*bvrTag = NULLTAG;

	try
	{
		tag_t bvTag = NULLTAG;
		tag_t t_BOMViewType=NULLTAG;
		
		ITK_CALL(iFail,PS_ask_default_view_type(&t_BOMViewType));

		GetItemBOMView1(itemTag,t_BOMViewType,&bvTag);
		if(bvTag ==NULLTAG)
		{
			//Item View does't exist so craete it.
			ITK_CALL2(iFail, AOM_refresh(itemTag, true),output,FAILETOCREATEBOMVIEWFORITEM);
			ITK_CALL2(iFail,PS_create_bom_view(t_BOMViewType, NULL, "BVR", itemTag, &bomview_tag),output,FAILETOCREATEBOMVIEWFORITEM);			
			ITK_CALL2(iFail,AOM_save(bomview_tag),output,FAILETOCREATEBOMVIEWFORITEM);
			ITK_CALL2(iFail, AOM_refresh(bomview_tag, false),output,FAILETOCREATEBOMVIEWFORITEM);
			ITK_CALL2(iFail, AOM_save(itemTag),output,FAILETOCREATEBOMVIEWFORITEM);
			ITK_CALL2(iFail,AOM_refresh(itemTag, false),output,FAILETOCREATEBOMVIEWFORITEM);
		}
		else
		{
			bomview_tag = bvTag;
		}
		if( bomview_tag != NULLTAG)
		{
			int intBVRcount=0;
			tag_t *tagBVRs=NULL;
			ITK_CALL(iFail,ITEM_rev_list_bom_view_revs(itemRevTag,&intBVRcount,&tagBVRs));
			if(intBVRcount>0)
			{
				//BVR already exist.
				*bvrTag=tagBVRs[0];
			}
			else
			{
				//Not exist create it
				ITK_CALL2(iFail, AOM_refresh(itemRevTag, true),output,FAILEDTOCREATEBVR);
				ITK_CALL2(iFail, PS_create_bvr(bomview_tag, NULL, "BVR",true, itemRevTag, bvrTag),output,FAILEDTOCREATEBVR);
				ITK_CALL2(iFail, AOM_save(*bvrTag),output,FAILEDTOCREATEBVR);
				ITK_CALL2(iFail, AOM_refresh(*bvrTag, false),output,FAILEDTOCREATEBVR);
				ITK_CALL2(iFail, AOM_save(itemRevTag),output,FAILEDTOCREATEBVR);
				ITK_CALL2(iFail, AOM_refresh(itemRevTag, false),output,FAILEDTOCREATEBVR);
			}
			ITK_free_memory(tagBVRs);

		
		}

	}
	catch(...)
	{
		output.isSuccess=false;
		output.strErrorList.push_back(FAILEDTOCREATEBVR);
	}
	return response;
}

int copy_part_bvr_to_mbom_bvr(tag_t tagEBOMRev, tag_t MBOMRev, Aii5::Soa::Altec::_2017_11::AltecTCExtension::SaveEZFormResponse &output)
{
	int iFail=ITK_ok;
	int     i                  = 0;
	int     part_child_count   = 0;
	tag_t   part_window        = NULLTAG;   
    tag_t   part_top_bom_line  = NULLTAG;
	tag_t   mbom_window        = NULLTAG;   
    tag_t   mbom_top_bom_line  = NULLTAG;
	tag_t   *part_child_line   = NULL;
	tag_t t_RevisionRule=NULLTAG;
	TC_write_syslog("In copy_part_bvr_to_mbom_bvr\n"); 


	ITK_CALL(iFail, BOM_create_window ( &part_window ));
	ITK_CALL(iFail,CFM_find("Precise Only",&t_RevisionRule)); // Need to get it from Preference.
	ITK_CALL(iFail,BOM_set_window_config_rule(part_window,t_RevisionRule));

    ITK_CALL(iFail, BOM_set_window_top_line  ( part_window , NULLTAG , tagEBOMRev , NULLTAG , &part_top_bom_line ));
    ITK_CALL(iFail,  BOM_line_ask_child_lines  ( part_top_bom_line, & part_child_count, &part_child_line )); 

	ITK_CALL(iFail, BOM_create_window ( &mbom_window ));
	ITK_CALL(iFail,BOM_set_window_config_rule(mbom_window,t_RevisionRule));
    ITK_CALL(iFail,  BOM_set_window_top_line  ( mbom_window , NULLTAG , MBOMRev , NULLTAG , &mbom_top_bom_line ));

	for( i = 0 ; i < part_child_count ; i++ )
	{
		tag_t   mbom_child_line  =  NULLTAG;
	

		 ITK_CALL2(iFail, BOM_line_copy  ( mbom_top_bom_line , part_child_line[i] , NULLTAG , &mbom_child_line ),output,FAILETOCOPYBOMLINE);

		
		
	}
	ITK_CALL2(iFail, BOM_save_window ( mbom_window ),output,FAILETOSAVEBOM);
	ITK_CALL(iFail, BOM_close_window ( mbom_window ));
	ITK_CALL(iFail,  BOM_close_window ( part_window ));
	ITK_free_memory( part_child_line );

	return iFail;

}

int reviseAction(tag_t tagEBOMRev,Aii5::Soa::Altec::_2017_11::AltecTCExtension::EZFormTableRow &EZFormRow,Aii5::Soa::Altec::_2017_11::AltecTCExtension::SaveEZFormResponse &output)
{
	int iFail=ITK_ok;

	string strOrgCode="";
	char *charItemID=NULL;
	string strItemID="";
	string strInfo="";
	if(!EZFormRow.strOrgID.empty()&& EZFormRow.strOrgID.length()>=3)
	{
		strOrgCode=EZFormRow.strOrgID.substr(0,3);
	}
	else
	{
		output.isSuccess=false;
		output.strErrorList.push_back(ORGCODEEMPTY);
	}
	
	ITK_CALL(iFail, AOM_ask_value_string(tagEBOMRev,ITEMID,&charItemID));
	if (charItemID!=NULL && !strOrgCode.empty())
	{
		strItemID=charItemID;
		strItemID+="_";
		strItemID+=strOrgCode;
	}
	if(!strItemID.empty())
	{
		tag_t tagMBOMItem=NULLTAG;
		tag_t tagMBOMItemRev=NULLTAG;
		ITK_CALL(iFail,ITEM_find_item(strItemID.c_str(),&tagMBOMItem));
		if(tagMBOMItem!=NULLTAG)
		{
			//Found MBOM
			int iReleasedCount=0;
			tag_t *ptReleaseStatus=NULL;
			ITK_CALL(iFail,ITEM_ask_latest_rev(tagMBOMItem,&tagMBOMItemRev));
			ITK_CALL(iFail,WSOM_ask_release_status_list(tagMBOMItemRev,&iReleasedCount,&ptReleaseStatus));
			ITK_free_memory(ptReleaseStatus);
			if(iReleasedCount>0 && is_date_pattern_matches(tagMBOMItemRev)==false)
			{
				tag_t revised_mbom_item_rev=NULLTAG;
				ITK_CALL(iFail,ITEM_copy_rev  ( tagMBOMItemRev , EZFormRow.strRev.c_str() , &revised_mbom_item_rev )); 
				if(revised_mbom_item_rev!=NULLTAG)
				{
					ITK_CALL2(iFail,AOM_refresh ( revised_mbom_item_rev, true ),output,FAILEDTOREVISE);
					ITK_CALL2(iFail,ITEM_save_rev( revised_mbom_item_rev ),output,FAILEDTOREVISE);
					ITK_CALL2(iFail,AOM_set_value_string  ( revised_mbom_item_rev , aii4_CopyBOMResult ,"N"),output,FAILEDTOREVISE); 
					ITK_CALL2(iFail, AOM_refresh ( revised_mbom_item_rev, false ),output,FAILEDTOREVISE);
					iFail=delete_relation_primary_to_secondary ( revised_mbom_item_rev  , ORGITEMRELATION ,output);
					iFail=create_relation_primary_to_secondary ( revised_mbom_item_rev , tagEBOMRev , ORGITEMRELATION ,output );
				}
				else
				{

				}

			}
			else
			{
				if(iReleasedCount>0)
				{
					output.isSuccess=false;
					output.strErrorList.push_back(MBOMSHOULDBERELEASED);
				}
				else
				{
					output.isSuccess=false;
					output.strErrorList.push_back(DATEPATTERNNOTMATCHED);
				}
			}
		}
		else
		{
			output.isSuccess=false;
			strInfo=strItemID;
			strInfo+=":";
			strInfo+=MBOMNOTFOUND;
			output.strErrorList.push_back(strInfo);
		}
	}
	else
	{
		output.isSuccess=false;
		output.strErrorList.push_back(NULLITEMID);
	}
	return iFail;
}
int createAction(tag_t tagEBOMRev,Aii5::Soa::Altec::_2017_11::AltecTCExtension::EZFormTableRow &EZFormRow,Aii5::Soa::Altec::_2017_11::AltecTCExtension::SaveEZFormResponse &output)
{
	int iFail=ITK_ok;
	tag_t rev_type_tag=NULLTAG;
	tag_t item_type_tag=NULLTAG;
	tag_t rev_create_input_tag=NULLTAG;
	tag_t item_create_input_tag=NULLTAG;
	tag_t mbom_item=NULLTAG;
	char *charItemID=NULL;
	string strItemID="";
	string strOrgCode="";
	string strEBOMID="";
	tag_t master_relation_type=NULLTAG;
	if(!EZFormRow.strOrgID.empty()&& EZFormRow.strOrgID.length()>=3)
	{
		strOrgCode=EZFormRow.strOrgID.substr(0,3);
	}
	else
	{
		output.isSuccess=false;
		output.strErrorList.push_back(ORGCODEEMPTY);
	}
	
	ITK_CALL(iFail, AOM_ask_value_string(tagEBOMRev,ITEMID,&charItemID));
	if (charItemID!=NULL && !strOrgCode.empty())
	{
		strItemID=charItemID;
		strEBOMID=charItemID;
		strItemID+="_";
		strItemID+=strOrgCode;

	}
	
	if(!strOrgCode.empty() && !strItemID.empty() && !EZFormRow.strRev.empty() &&!EZFormRow.strInitialTemplate.empty() )
	{
		ITK_CALL(iFail, TCTYPE_find_type (ORGITEMREV , NULL , &rev_type_tag ));	
		ITK_CALL(iFail, TCTYPE_find_type(ORGITEM , NULL , &item_type_tag ));	

		ITK_CALL(iFail, TCTYPE_construct_create_input( item_type_tag , &item_create_input_tag ));
		ITK_CALL(iFail, TCTYPE_construct_create_input ( rev_type_tag , &rev_create_input_tag ));
		
		ITK_CALL(iFail, AOM_set_value_string  ( rev_create_input_tag , ITEMREVISIONID , EZFormRow.strRev.c_str() )); 
		ITK_CALL(iFail, AOM_set_value_string  ( rev_create_input_tag , ORGCODE , EZFormRow.strOrgID.c_str() ));
			   
		
		ITK_CALL(iFail, AOM_set_value_string  ( item_create_input_tag ,ITEMID , strItemID.c_str() )); 
		ITK_CALL(iFail, AOM_set_value_tag( item_create_input_tag , REVISION , rev_create_input_tag ));
		ITK_CALL2(iFail, TCTYPE_create_object( item_create_input_tag , &mbom_item ),output,FAILEDTOCREATEITEM);
		ITK_CALL2(iFail, AOM_save( mbom_item ),output,FAILEDTOCREATEITEM);
	//	ITK_CALL(iFail, AOM_refresh(mbom_item,false));
		ITK_CALL(iFail, GRM_find_relation_type	(IMAN_master_form,&master_relation_type));
		int count=0;
		tag_t *master_form=NULL;
		tag_t mbom_item_rev=NULLTAG;
		ITK_CALL(iFail,ITEM_ask_latest_rev(mbom_item,&mbom_item_rev));
		//ITK_CALL(iFail, GRM_list_secondary_objects_only( mbom_item , master_relation_type , &count , &master_form ));
		
	//	if(count>0)
	//	{
			ITK_CALL(iFail,AOM_refresh( mbom_item_rev,true ));
			string strZoomTemplate=get_zoom_template_internal_value(EZFormRow.strInitialTemplate);
			
			ITK_CALL(iFail,AOM_set_value_string(mbom_item_rev , aii4_ZoomTemplate ,strZoomTemplate.c_str()));
			if(iFail!=ITK_ok)
			{
				char *err_string=NULL;
				EMH_ask_error_text (iFail, &err_string); 
				if(err_string!=NULL)
				{
					output.isSuccess=false;
					output.strErrorList.push_back(err_string);
				}
				ITK_free_memory(err_string);
			}
			ITK_CALL2(iFail,AOM_save( mbom_item_rev ),output,FAILEDTOCREATEITEM);
			if(iFail!=ITK_ok)
			{
				char *err_string=NULL;
				EMH_ask_error_text (iFail, &err_string); 
				if(err_string!=NULL)
				{
					output.isSuccess=false;
					output.strErrorList.push_back(err_string);
				}
				ITK_free_memory(err_string);
			}
			ITK_CALL(iFail,AOM_refresh( mbom_item_rev,false ));
	//	}
		//ITK_free_memory(master_form);
		
		if(tc_strcmp(EZFormRow.strCopyBOM.c_str(),"Y")==0 && EZFormRow.strCommonFrom.empty())
		{
			//Copy BOM/
			//Create BVR
			
			
			if(mbom_item_rev!=NULLTAG)
			{
				int intBVRcount=0;
				tag_t *tagBVRs=NULL;
				ITK_CALL(iFail,ITEM_rev_list_bom_view_revs(tagEBOMRev,&intBVRcount,&tagBVRs));
				if(intBVRcount>0)
				{
					tag_t tagBVR=NULLTAG;
					iFail=CheckBVRExist1(mbom_item,mbom_item_rev,&tagBVR,output);
					iFail=copy_part_bvr_to_mbom_bvr(tagEBOMRev,mbom_item_rev,output);	
					
				}
				ITK_free_memory(tagBVRs);
				
				
				iFail=create_relation_primary_to_secondary(mbom_item_rev,tagEBOMRev,ORGITEMRELATION,output);
			}
			//Update MBOM
			ITK_CALL(iFail,AOM_refresh(mbom_item_rev, true));  
			ITK_CALL(iFail,AOM_set_value_string  ( mbom_item_rev , aii4_CopyBOMResult ,"Y New")); 
			ITK_CALL(iFail, AOM_save  ( mbom_item_rev));
			TC_write_syslog("setCopyBomResult set Y New\n"); 
			ITK_CALL(iFail,AOM_refresh(mbom_item_rev, false));  
			
			

		}

		else if(tc_strcmp(EZFormRow.strCopyBOM.c_str(),"N")==0  && EZFormRow.strCommonFrom.empty())
		{
			iFail=create_relation_primary_to_secondary(mbom_item_rev,tagEBOMRev,ORGITEMRELATION,output);
			ITK_CALL(iFail,AOM_refresh(mbom_item_rev, true));  
			ITK_CALL(iFail,AOM_set_value_string  ( mbom_item_rev , aii4_CopyBOMResult ,"N New")); 
			ITK_CALL(iFail, AOM_save  ( mbom_item_rev));
			TC_write_syslog("setCopyBomResult set N New\n"); 
			ITK_CALL(iFail,AOM_refresh(mbom_item_rev, false));  

		}
		else if(!EZFormRow.strCommonFrom.empty())
		{
			//Common BOM
			//Create BVR
			
			
			//tag_t mbom_item_rev=NULLTAG;
			
		//	ITK_CALL(iFail,ITEM_ask_latest_rev(mbom_item,&mbom_item_rev));
			iFail=create_relation_primary_to_secondary(mbom_item_rev,tagEBOMRev,ORGITEMRELATION,output);
			
			
			//Update MBOM
			string strCommonMBOMID="";
			if(!strEBOMID.empty())
			{
				string strCommonFromOrgCode=EZFormRow.strCommonFrom.substr(0,3);
				strCommonMBOMID=strEBOMID;
				strCommonMBOMID+="_";
				strCommonMBOMID+=strCommonFromOrgCode;
			}
			tag_t tagCommonFromItemRev=NULLTAG;
			tag_t tagCommonFromItem=NULLTAG;
			ITK_CALL(iFail,ITEM_find_item(strCommonMBOMID.c_str(),&tagCommonFromItem));
			
			if(tagCommonFromItem!=NULLTAG)
			{
				ITK_CALL(iFail,ITEM_ask_latest_rev(tagCommonFromItem,&tagCommonFromItemRev));
				if(mbom_item_rev!=NULLTAG && tagCommonFromItemRev!=NULLTAG)
				{
					tag_t tagBVR=NULLTAG;
					iFail=CheckBVRExist1(mbom_item,mbom_item_rev,&tagBVR,output);
					iFail=copy_mbomrev_to_mbom_bvr(mbom_item_rev,tagCommonFromItemRev,output);
				}
				
				
				
			}
			else
			{
				output.isSuccess=false;
				output.strErrorList.push_back(COMMONFROMBOMNONTFOUND);
			}
			
			
		}
		else
		{
			TC_write_syslog("Create/Update MBOM\n");
		}
		

	}
	else
	{
		output.strErrorList.push_back(MANDATORYINPUTMISSING);
		TC_write_syslog("SOA Input Error:Mandatory Input missing");
	}
	ITK_free_memory(charItemID);

	return iFail;
}

EXTERN_DLL Aii5::Soa::Altec::_2017_11::AltecTCExtension::SaveEZFormResponse altecsaveEZFormData( Aii5::Soa::Altec::_2017_11::AltecTCExtension::SaveEZFormInput altecSaveEZFormInput )
{
	Aii5::Soa::Altec::_2017_11::AltecTCExtension::SaveEZFormResponse output;
	TC_write_syslog("EZ Form save begin");
	int iFail=ITK_ok;
	tag_t tagEBOMRev=altecSaveEZFormInput.selectedObject.tag();
	if(tagEBOMRev!=NULLTAG)
	{
		output.isSuccess=true; //Default value 
		int MarkPoint=1;
		logical	LogicalMarkPoint;
		ITK_CALL(iFail,POM_place_markpoint(&MarkPoint)); //Creation of MarkPoint 
		for (int i=0;i<altecSaveEZFormInput.vectorEZFormTableRows.size();i++)
		{
			string strInfo="";
			Aii5::Soa::Altec::_2017_11::AltecTCExtension::EZFormTableRow EZFormRow=altecSaveEZFormInput.vectorEZFormTableRows[i];
			if(tc_strcmp(EZFormRow.strAction.c_str(),CREATE)==0)
			{
				strInfo="EZ Form ::Create Action Start::"+ EZFormRow.strOrgID;
				TC_write_syslog(strInfo.c_str());
				iFail=createAction(tagEBOMRev,EZFormRow,output);
				strInfo="EZ Form ::Create Action Start::"+ EZFormRow.strOrgID;
				TC_write_syslog(strInfo.c_str());
			}
			else if(tc_strcmp(EZFormRow.strAction.c_str(),REVISE)==0)
			{
				strInfo="EZ Form ::Revise Action Start::"+ EZFormRow.strOrgID;
				TC_write_syslog(strInfo.c_str());
				iFail=reviseAction(tagEBOMRev,EZFormRow,output);
				strInfo="EZ Form ::Revise Action Start::"+ EZFormRow.strOrgID;
				TC_write_syslog(strInfo.c_str());
			}
			else
			{
				//No Action
				strInfo="EZ Form ::No Action::"+ EZFormRow.strOrgID;
				TC_write_syslog(strInfo.c_str());
							
			}

		}
		if(output.isSuccess==false)
		{
			//Rollback
			ITK_CALL(iFail,POM_roll_to_markpoint(MarkPoint, &LogicalMarkPoint));
		}
	}
	else
	{
		output.strErrorList.push_back(EBOMNULLERROR);
		output.isSuccess=false;
	}
	
	
	TC_write_syslog("EZ Form save begin");
	return output;
}