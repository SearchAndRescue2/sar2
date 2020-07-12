jQuery(document).ready(function(){
    jQuery('#tabs').tabs({
	show: function(event, ui) {
	    window.location.hash=ui.panel.id;
	}
    });
    
    jQuery(".ui-tabs .ui-tabs-nav").css({textAlign: 'center'});
    jQuery(".ui-tabs .ui-tabs-nav li").css({float: 'none', display: 'inline-block'});
    jQuery(".ui-tabs .ui-tabs-nav li a").css({float: 'none', display: 'inline-block'});
    
    jQuery('#hidden_news').hide();

    var span1 = '<span class="ui-icon ui-icon-triangle-1-e" style="display:inline-block;vertical-align:middle;"></span>';

    var span2 = '<span class="ui-icon ui-icon-triangle-1-s" style="display:inline-block;vertical-align:middle;"></span>';

    jQuery('#show_hide').html('More '+span1);

    jQuery('#show_hide').toggle(function(){
        jQuery('#hidden_news').fadeIn();
        jQuery(this).html('Less '+span2);
        return false;
    },function(){
        jQuery('#hidden_news').fadeOut();
        jQuery(this).html('More '+span1);
        return false;
    });

});
