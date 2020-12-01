/**
* \file
* \lib siemens.dll
* \since Release R4
* \env C,ITK
* \ingroup workflow_action_handlers
* \brief    This defines an action handler to get the latest inheritance from Source Material.
*/

/*------------------------------------------------------------------------------
* History
*---------------------------------------------------------------------------------------------------------------------------
* Date          Name                 Description of Change
* 11-Oct-2016   Renjith George       P5A00190282-Material Release: adapt "MDS Attach Latest Revision" handler for IMM.
* ------------------------------------------------------------------------------
*
* ------------------------------------------------------------------------------ */

#ifdef _WIN32
#include <iostream>
#include <direct.h>
#else
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#endif

#include "siemens_workflow.h"
#include "siemens_trace_handling.h"
#include <stdio.h>

/**
\ingroup workflow_action_handlers

\par Description :
\verbatim
This defines an action handler to get the latest inheritance from Source Material.

\endverbatim

\par Placement :
Place on complete action of the task.

\par Restrictions :

\par Usage in Workflows : 
Material Release workflows

\param[in] msg :        Message of the handler

\par Returns :
int : ITK_ok / error code

*/

/*------------------------------------------------------------------------------
* History
*---------------------------------------------------------------------------------------------------------------------------
* Date          Name                 Description of Change
* 11-Oct-2016   Renjith George       P5A00190282-Material Release: adapt "MDS Attach Latest Revision" handler for IMM.
* ------------------------------------------------------------------------------
* ------------------------------------------------------------------------------ */

int siemens_search_extends_material_revision( char* itemID, char* revID, tagList_t& lMatRevList );

extern "C" int
    siemens_imm_attach_latest_revision( EPM_action_message_t msg )
{
    int iStatus                     = ITK_ok;
    int iRefAttCount                = 0;
    tag_t* tagRefAtts               = NULL;
    tagList_t lMatRevList;
    char cObjecttype[TCTYPE_name_size_c + 1]  = "";

    const char* __function__ = "siemens_imm_attach_latest_revision" ;
    SIEMENS_TRACE_ENTER();

    try
    {
        SIEMENS_TRACE_CALL( iStatus = Siemens_ask_attachments_from_task( msg.task, EPM_target_attachment, &iRefAttCount, &tagRefAtts ) );
        SIEMENS_LOG_ERROR_AND_THROW_STATUS;

        //If attachments are not NULL then only proceed.
        if( iRefAttCount > 0 )
        {
            //Get object type
            for( int attCount = 0; attCount < iRefAttCount; attCount++ )
            {
                SIEMENS_TRACE_CALL( iStatus = Siemens_ask_object_type( tagRefAtts[attCount], cObjecttype ) );
                SIEMENS_LOG_ERROR_AND_THROW_STATUS;

                if( tc_strcmp( cObjecttype, MAT1MATERIALREVISION ) == 0 )
                {
                    char* pItemId = NULL;
                    char* pItemRevId = NULL; // Remove RevID
                    SIEMENS_TRACE_CALL( iStatus = AOM_ask_value_string( tagRefAtts[attCount], SIEMENS_ATTRIBUTE_ITEM_ID, &pItemId ) );
                    SIEMENS_LOG_ERROR_AND_THROW_STATUS;
                    SIEMENS_TRACE_CALL( iStatus = AOM_ask_value_string( tagRefAtts[attCount], SIEMENS_ATTRIBUTE_ITEM_REV_ID, &pItemRevId ) );
                    SIEMENS_LOG_ERROR_AND_THROW_STATUS;

                    SIEMENS_TRACE_CALL( iStatus = siemens_search_extends_material_revision( pItemId, pItemRevId, lMatRevList ) );
                    SIEMENS_LOG_ERROR_AND_THROW_STATUS;

                    logical lWasLockedBefore = false;

                    for( tagList_Iter_t itr = lMatRevList.begin(); itr != lMatRevList.end(); itr++ )
                    {
                        SIEMENS_TRACE_CALL( AM__set_application_bypass( true ) );
                        SIEMENS_TRACE_CALL( POM_AM__set_application_bypass( true ) );

                        SIEMENS_TRACE_CALL( iStatus = POM_modifiable( *itr, &lWasLockedBefore ) ) ;
                        SIEMENS_LOG_ERROR_AND_THROW_STATUS;

                        if( !lWasLockedBefore )
                        {
                            SIEMENS_TRACE_CALL( iStatus = AOM_refresh( *itr, TRUE ) ) ;
                            SIEMENS_LOG_ERROR_AND_THROW_STATUS;
                        }

                        SIEMENS_TRACE_CALL( iStatus = AOM_set_value_tag( *itr, "mat1ExtendsMaterial", tagRefAtts[attCount] ) );
                        SIEMENS_LOG_ERROR_AND_THROW_STATUS;
                        SIEMENS_TRACE_CALL( iStatus = AOM_save_with_extensions( *itr ) );
                        SIEMENS_LOG_ERROR_AND_THROW_STATUS;

                        if( !lWasLockedBefore )
                        {
                            SIEMENS_TRACE_CALL( iStatus = AOM_refresh( *itr, FALSE ) ) ;
                            SIEMENS_LOG_ERROR_AND_THROW_STATUS;
                        }
                        SIEMENS_TRACE_CALL( AM__set_application_bypass( false ) );
                        SIEMENS_TRACE_CALL( POM_AM__set_application_bypass( false ) );
                    } // end of for loop for all primary material revisions
                    Custom_free(pItemId);
                    Custom_free(pItemRevId);
                } // end of if Mat1Material Revision
            }
        }
    }
    catch( ... )
    {
        SIEMENS_TRACE_CALL( AM__set_application_bypass( false ) );
    }

    Custom_free( tagRefAtts );
    SIEMENS_TRACE_LEAVE_RVAL( "%d", iStatus );
    return  ITK_ok ;
}


/*****************************************************************************************************
 * Function Name    : siemens_search_extends_material_revision
 * Description      : This function queries for Mat1Material Revs having the extends Material
 * RETURN VALUE     : int :ITK_ok/error code
 * GLOBALS USED     :
 * FUNCTIONS CALLED :
 *******************************************************************************************************/
int siemens_search_extends_material_revision( char* itemID, char* revID, tagList_t& lMatRevList )
{
    int iStatus                     = ITK_ok;
    int iTasksCount                 = 0;
    int index                       = 0;
    tag_t tQry                      = NULLTAG;
    tag_t* pResult                  = NULL;
    char**   pcEntries              = NULL;
    char**   pcValues               = NULL;
    strList_t lAttribs;
    strList_t lAttribVals;

    const char* __function__ = "siemens_search_extends_material_revision";
    SIEMENS_TRACE_ENTER();

    try
    {
        std::string szQueryName( "" );
        szQueryName.assign( "__FindExtendsMaterial" );

        SIEMENS_TRACE_CALL( iStatus = QRY_find2( szQueryName.c_str(), &tQry ) );
        SIEMENS_LOG_ERROR_AND_THROW_STATUS;

        // check if saved qry does exist or not
        if( tQry == NULLTAG )
        {
            iStatus = SIEMENS_SAVED_QRY_NOT_FOUND;
            EMH_store_error_s1( EMH_severity_error, iStatus, szQueryName.c_str() );
            return iStatus;
        }

        if( tQry != NULLTAG )
        {
            lAttribs.clear();
            lAttribVals.clear();
            lAttribs.push_back("ID");
            lAttribVals.push_back(itemID);

            //lAttribs.push_back("Revision");
            //lAttribVals.push_back("");

            //Allocate memory to query entries and values
            int numOfEntries = ( int )lAttribs.size();
            pcEntries = ( char** ) MEM_alloc( numOfEntries * sizeof( char* ) );
            pcValues = ( char** ) MEM_alloc( numOfEntries * sizeof( char* ) );

            //Copy the Entries from Query, into "char *"
            index = 0;
            for( strList_Iter_t ListIter = lAttribs.begin();ListIter != lAttribs.end();ListIter++ )
            {
                pcEntries[index] = ( char* )MEM_alloc( ( tc_strlen( ( *ListIter ).c_str() ) + 1 )  * sizeof( char ) );
                tc_strcpy( pcEntries[index], ( *ListIter ).c_str() );
                index++;
            }

            //Copy the Values for Entries, into "char *"
            index = 0;
            for( strList_Iter_t ListIter1 = lAttribVals.begin();ListIter1 != lAttribVals.end();ListIter1++ )
            {
                pcValues[index] = ( char* )MEM_alloc( ( tc_strlen( ( *ListIter1 ).c_str() ) + 1 ) * sizeof( char ) );
                tc_strcpy( pcValues[index], ( *ListIter1 ).c_str() );
                index++;
            }
            //Execute query
            SIEMENS_TRACE_CALL( iStatus = QRY_execute( tQry, numOfEntries, pcEntries, pcValues, &iTasksCount, &pResult ) );
            SIEMENS_LOG_ERROR_AND_THROW_STATUS;

            if( iTasksCount > 0 )
            {
                for( int iTskInx = 0; iTskInx < iTasksCount; iTskInx++ )
                {
                    lMatRevList.push_back( pResult[iTskInx] );
                }
            }
        }
    }// end try
    catch( ... )
    {
        Custom_free( pcEntries );
        Custom_free( pcValues );
        Custom_free( pResult );
        SIEMENS_TRACE_ARGS( ( "\n Exception Caught %d", iStatus ) );
    }

    Custom_free( pcEntries );
    Custom_free( pcValues );
    Custom_free( pResult );
    return iStatus;
}