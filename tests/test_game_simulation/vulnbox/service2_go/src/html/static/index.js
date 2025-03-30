
$('#btn_get_flag').unbind().bind('click', function() {
    $('#form_put_flag').hide();
    $('#form_list_flag').hide();
    $('#form_get_flag').show();
})

$('#btn_put_flag').unbind().bind('click', function() {
    $('#form_get_flag').hide();
    $('#form_list_flag').hide();
    $('#form_put_flag').show();

});

$('#btn_list_flag').unbind().bind('click', function() {
    $('#form_get_flag').hide();
    $('#form_put_flag').hide();
    $('#form_list_flag').show();

    $.ajax({
        url: '/api/flags/',
        type: 'GET'
    }).done(function(r){
        $('#tbl_list_flag').html("<tr><th>Flag ID</th></tr>");
        for(var i in r.FlagIDs) {
            $('#tbl_list_flag').append("<tr><td class='flag-id'>" + r.FlagIDs[i] + "</td></tr>");
        }
    }).fail(function(err){
        $('#tbl_list_flag').html("<tr><th>" + err.responseJSON.Error + "</th></tr>");
    })
});

$('#btn_find_flag').unbind().bind('click', function() {
    var flag_id = $('#get_flag_id').val();
    $.ajax({
        url: '/api/flags/' + flag_id,
        type: 'GET'
    }).done(function(r){
        $('#result_find_flag').html(r.Flag);
    }).fail(function(err){
        $('#result_find_flag').html(err.responseJSON.Error);
    })
})

$('#btn_insert_flag').unbind().bind('click', function() {
    var flag_id = $('#put_flag_id').val();
    var flag = $('#put_flag').val();
    $.ajax({
        url: '/api/flags/' + flag_id + "/" + flag,
        type: 'POST'
    }).done(function(r){
        $('#result_insert_flag').html(r.Flag);
    }).fail(function(err){
        $('#result_insert_flag').html(err.responseJSON.Error);
    })
})