<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="utf-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <meta name="description" content="">
    <meta name="author" content="">
	<link rel="shortcut icon" href="favicon.ico" type="image/x-icon" />

    <title>Visual FAIL*</title>

    <!-- Bootstrap core CSS -->
    <link href="css/bootstrap.css" rel="stylesheet">

    <!-- Custom styles for this template -->
    <link href="css/myStyle.css" rel="stylesheet">

    <!-- Just for debugging purposes. Don't actually copy this line! -->
    <!--[if lt IE 9]><script src="../../docs-assets/js/ie8-responsive-file-warning.js"></script><![endif]-->

    <!-- HTML5 shim and Respond.js IE8 support of HTML5 elements and media queries -->
    <!--[if lt IE 9]>
      <script src="https://oss.maxcdn.com/libs/html5shiv/3.7.0/html5shiv.js"></script>
      <script src="https://oss.maxcdn.com/libs/respond.js/1.3.0/respond.min.js"></script>
    <![endif]-->
  </head>

  <body>

    <!-- Fixed navbar -->
	<div class="container-full">
		<div class="navbar navbar-default navbar-fixed-top" role="navigation">
			<div class="navbar-header">
				<button type="button" class="navbar-toggle" data-toggle="collapse" data-target=".navbar-collapse">
					<span class="sr-only">Toggle navigation</span>
					<span class="icon-bar"></span>
					<span class="icon-bar"></span>
					<span class="icon-bar"></span>
				</button>
				<!--<div class="container"> -->
				  <!--<a class="navbar-brand" href="#">Visual FAIL*</a>-->
				  <img class="navbar-brand logo" src="images/logos/visualfaillogo_klein.png" alt="Visual FAIL* logo" />
				<!--</div>-->
			</div>
			<div class="container">
				<div class="navbar-collapse collapse">
				  <ul class="nav navbar-nav">
					<li class="active"><a href="#">Result Mapping</a></li>
					<li><a href="#about">Aggregation</a></li>
<!--
					<li class="dropdown">
					  <a href="#" class="dropdown-toggle" data-toggle="dropdown">Tools <b class="caret"></b></a>
					  <ul class="dropdown-menu">
						<li><a href="adminer-3.7.1.php" target="_blank">Adminer</a></li>
					  </ul>
					</li>
-->
				  </ul>
				</div><!--/.nav-collapse -->
			</div>
		</div>

		<div class="row">
			<div class="col-md-2">
			  <!-- Main component for a primary marketing message or call to action -->
				<div class="panel panel-default">
					<div class="panel-heading">Coloring</div>
						<div class="panel-body">
							<select class="form-control" id="faerbung">
							  <option value="onlyRightEdge">Right margin (trace.instr2)</option>
							  <option value="latestip">Last instruction (result.latest_ip)</option>
							  <option value="normal">Equivalence classes</option>
							</select>
						</div>
				</div>

				<div class="panel panel-default">
					<div class="panel-heading">Benchmark</div>
						<div class="panel-body">
							<select class="form-control" id="binary">
							  <option></option>
							</select>
						</div>
				</div>

				<div class="panel panel-default">
					<div class="panel-heading">Variant</div>
						<div class="panel-body">
							<select class="form-control" id="variant">
							  <option></option>
							</select>
							<div class="text-center">
								<button type="button" class="btn btn-default btn-lg" id="analyse">Analysis</button>
							</div>
						</div>
				</div>
			</div>

			<div class="col-md-10">
						<div class="panel-body">
							<div class="row">
								<div class="col-md-6">
									<div class="btn-group" id="fehlertypenset" data-toggle="buttons">
									</div>
								</div>
								<div class="col-md-6">
									<select class="form-control" id="sourceFiles">
										<option></option>
									</select>
								</div>
							</div>
						</div>
					<div class="row">
						<pre class="container col-md-6" id="asm">
						</pre>
						<pre class="container col-md-6" id="hcode">
						</pre>
					</div>
			</div>
		</div>
	</div> <!-- /container-full -->

    <!-- Bootstrap core JavaScript
    ================================================== -->
    <!-- Placed at the end of the document so the pages load faster -->
    <script src="https://code.jquery.com/jquery-1.10.2.min.js"></script>
    <script src="js/bootstrap.min.js"></script>
	<script src="js/myscript.js"></script>
  </body>
</html>
