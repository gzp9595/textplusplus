$(document).ready(function() {
    
    $.get("api/semantic/classify", function(data) {
        var classify_data = eval("(" + data + ")");
        var classify_data_array = new Array();
        var classification = new Array();
        var possibility = new Array();
        classify_data_array = classify_data.classify_data;
        for (var i = 0; i < classify_data.classify_data.length; i++) {
            classification[i] = classify_data.classify_data[i].classification;
            possibility[i] = parseInt(classify_data.classify_data[i].possibility);

        }
        draw_analysis_chart();
    });

    function draw_classify_chart() {
    $(function () {
        Highcharts.setOptions({
            colors: ['rgb(74,211,200)']
        });
        // Create the chart
        $('#classify_container').highcharts({
            chart: {
                type: 'column',
                backgroundColor: 'rgba(0,0,0,0)',
            },
            title: {
                text: '文本分类'
            },
            credits: {
                enabled: false
            },
            exporting: {
                enabled: false
            },
            xAxis: {
                categories: classification
            },
            yAxis: {
                title: {
                    text: '可能性'
                }

            },
            legend: {
                enabled: false,
            },
            plotOptions: {
                series: {
                    borderWidth: 0,
                    dataLabels: {
                        enabled: true,
                        format: '{point.y:.1f}%'
                    }
                }
            },

            tooltip: {
                headerFormat: '<span style="font-size:11px">{series.name}</span><br>',
                pointFormat: '<span style="color:{point.color}">{point.name}</span>: <b>{point.y:.2f}%</b> of total<br/>'
            },

            series: [{
                name: 'Brands',
                colorByPoint: false,
                data: [{
                    name: classification[0],
                    y: possibility[0],
                   
                }, {
                    name: classification[1],
                    y: possibility[1],
                   
                }, {
                    name: classification[2],
                    y: possibility[2],
                    
                }, {
                    name: classification[3],
                    y: possibility[3],
                   
                }, {
                    name: classification[4],
                    y: possibility[4],
                  
                }, {
                    name: classification[5],
                    y: possibility[5],
                    
                }]
            }],
            
        });
    });
    }
})